#include "BigInt.hpp"
#include <algorithm>
#include <bit>
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <string>

BigInt::BigInt(int64_t val) {
  isNegative = (val < 0);
  // two's complement negation in unsigned arithmetic is well defined for
  // every int64_t, including INT64_MIN
  uint64_t absVal = isNegative ? 0 - static_cast<uint64_t>(val)
                               : static_cast<uint64_t>(val);

  limbs.clear();
  do {
    limbs.push_back(static_cast<uint32_t>(absVal & 0xFFFFFFFF));
    absVal >>= 32;
  } while (absVal > 0);
}

namespace {
uint32_t HexDigitValue(char c) {
  if (c >= '0' && c <= '9')
    return static_cast<uint32_t>(c - '0');
  if (c >= 'a' && c <= 'f')
    return static_cast<uint32_t>(c - 'a' + 10);
  if (c >= 'A' && c <= 'F')
    return static_cast<uint32_t>(c - 'A' + 10);
  throw std::invalid_argument("BigInt: invalid hex digit");
}
} // namespace

BigInt::BigInt(const std::string &hexStr) {
  size_t startIdx = hexStr.starts_with('-') ? 1 : 0;
  if (hexStr.size() == startIdx)
    throw std::invalid_argument("BigInt: empty hex string");

  limbs.clear();
  // parse right to left, 8 hex digits (32 bits) per limb, so limbs end up in
  // little-endian order
  for (size_t end = hexStr.size(); end > startIdx;) {
    size_t chunkStart = end - startIdx > 8 ? end - 8 : startIdx;
    uint32_t limbVal = 0;
    for (size_t i = chunkStart; i < end; ++i)
      limbVal = (limbVal << 4) | HexDigitValue(hexStr[i]);
    limbs.push_back(limbVal);
    end = chunkStart;
  }

  isNegative = (startIdx == 1);
  trim();
}

void BigInt::trim() {
  while (limbs.size() > 1 && limbs.back() == 0) {
    limbs.pop_back();
  }

  if (limbs.size() == 1 && limbs[0] == 0) {
    isNegative = false;
  }
}

namespace {
using Mag = std::vector<uint32_t>;

void TrimMag(Mag &m) {
  while (m.size() > 1 && m.back() == 0)
    m.pop_back();
}

// -1, 0, 1; both inputs trimmed
int CompareMag(const Mag &a, const Mag &b) {
  if (a.size() != b.size())
    return a.size() < b.size() ? -1 : 1;
  for (size_t i = a.size(); i-- > 0;)
    if (a[i] != b[i])
      return a[i] < b[i] ? -1 : 1;
  return 0;
}

Mag AddMag(const Mag &a, const Mag &b) {
  const Mag &big = a.size() >= b.size() ? a : b;
  const Mag &small = a.size() >= b.size() ? b : a;
  Mag r;
  r.reserve(big.size() + 1);
  uint64_t carry = 0;
  for (size_t i = 0; i < big.size(); ++i) {
    uint64_t sum = carry + big[i] + (i < small.size() ? small[i] : 0);
    r.push_back(static_cast<uint32_t>(sum));
    carry = sum >> 32;
  }
  if (carry)
    r.push_back(static_cast<uint32_t>(carry));
  return r;
}

// requires |a| >= |b|
Mag SubMag(const Mag &a, const Mag &b) {
  Mag r;
  r.reserve(a.size());
  int64_t borrow = 0;
  for (size_t i = 0; i < a.size(); ++i) {
    int64_t diff = static_cast<int64_t>(a[i]) - borrow -
                   (i < b.size() ? static_cast<int64_t>(b[i]) : 0);
    borrow = diff < 0 ? 1 : 0;
    r.push_back(static_cast<uint32_t>(diff)); // wraps mod 2^32
  }
  TrimMag(r);
  return r;
}

Mag MulMag(const Mag &a, const Mag &b) {
  Mag r(a.size() + b.size(), 0);
  for (size_t i = 0; i < a.size(); ++i) {
    uint64_t carry = 0;
    for (size_t j = 0; j < b.size(); ++j) {
      uint64_t cur = static_cast<uint64_t>(a[i]) * b[j] + r[i + j] + carry;
      r[i + j] = static_cast<uint32_t>(cur);
      carry = cur >> 32;
    }
    r[i + b.size()] = static_cast<uint32_t>(carry);
  }
  TrimMag(r);
  return r;
}

Mag ShiftLeftMag(const Mag &a, size_t bits) {
  size_t limbShift = bits / 32;
  unsigned bitShift = bits % 32;
  Mag r(a.size() + limbShift + 1, 0);
  for (size_t i = 0; i < a.size(); ++i) {
    uint64_t v = static_cast<uint64_t>(a[i]) << bitShift;
    r[i + limbShift] |= static_cast<uint32_t>(v);
    r[i + limbShift + 1] |= static_cast<uint32_t>(v >> 32);
  }
  TrimMag(r);
  return r;
}

Mag ShiftRightMag(const Mag &a, size_t bits) {
  size_t limbShift = bits / 32;
  unsigned bitShift = bits % 32;
  if (limbShift >= a.size())
    return Mag{0};
  Mag r(a.size() - limbShift, 0);
  for (size_t i = 0; i < r.size(); ++i) {
    uint64_t v = a[i + limbShift];
    if (i + limbShift + 1 < a.size())
      v |= static_cast<uint64_t>(a[i + limbShift + 1]) << 32;
    r[i] = static_cast<uint32_t>(v >> bitShift);
  }
  TrimMag(r);
  return r;
}

// Knuth TAOCP vol.2 4.3.1 Algorithm D. v must be nonzero. Inputs trimmed.
void DivModMag(const Mag &u, const Mag &v, Mag &q, Mag &r) {
  if (CompareMag(u, v) < 0) {
    q = Mag{0};
    r = u;
    return;
  }
  if (v.size() == 1) {
    q.assign(u.size(), 0);
    uint64_t rem = 0;
    for (size_t i = u.size(); i-- > 0;) {
      uint64_t cur = (rem << 32) | u[i];
      q[i] = static_cast<uint32_t>(cur / v[0]);
      rem = cur % v[0];
    }
    TrimMag(q);
    r = Mag{static_cast<uint32_t>(rem)};
    return;
  }

  const size_t n = v.size();
  const size_t m = u.size() - n;
  const unsigned s = static_cast<unsigned>(std::countl_zero(v.back()));

  // normalize so the top bit of the divisor is set
  Mag vn = ShiftLeftMag(v, s);
  vn.resize(n);
  Mag un = ShiftLeftMag(u, s);
  un.resize(u.size() + 1, 0);

  constexpr uint64_t B = uint64_t{1} << 32;
  q.assign(m + 1, 0);
  for (size_t j = m + 1; j-- > 0;) {
    uint64_t num = (static_cast<uint64_t>(un[j + n]) << 32) | un[j + n - 1];
    uint64_t qhat = num / vn[n - 1];
    uint64_t rhat = num % vn[n - 1];
    while (qhat >= B ||
           qhat * vn[n - 2] > ((rhat << 32) | un[j + n - 2])) {
      --qhat;
      rhat += vn[n - 1];
      if (rhat >= B)
        break;
    }

    // multiply and subtract: un[j..j+n] -= qhat * vn
    int64_t k = 0;
    int64_t t;
    for (size_t i = 0; i < n; ++i) {
      uint64_t p = qhat * vn[i];
      t = static_cast<int64_t>(un[i + j]) - k -
          static_cast<int64_t>(p & 0xFFFFFFFF);
      un[i + j] = static_cast<uint32_t>(t);
      k = static_cast<int64_t>(p >> 32) - (t >> 32);
    }
    t = static_cast<int64_t>(un[j + n]) - k;
    un[j + n] = static_cast<uint32_t>(t);

    q[j] = static_cast<uint32_t>(qhat);
    if (t < 0) { // qhat was one too large: add the divisor back
      --q[j];
      uint64_t carry = 0;
      for (size_t i = 0; i < n; ++i) {
        uint64_t sum = static_cast<uint64_t>(un[i + j]) + vn[i] + carry;
        un[i + j] = static_cast<uint32_t>(sum);
        carry = sum >> 32;
      }
      un[j + n] += static_cast<uint32_t>(carry);
    }
  }
  TrimMag(q);
  un.resize(n);
  r = ShiftRightMag(un, s);
}
} // namespace

std::strong_ordering BigInt::operator<=>(const BigInt &other) const {
  if (isNegative != other.isNegative)
    return isNegative ? std::strong_ordering::less
                      : std::strong_ordering::greater;
  int c = CompareMag(limbs, other.limbs);
  if (isNegative)
    c = -c;
  return c < 0   ? std::strong_ordering::less
         : c > 0 ? std::strong_ordering::greater
                 : std::strong_ordering::equal;
}

void BigInt::AddSigned(const BigInt &other, bool otherNegative) {
  if (&other == this) {
    BigInt copy = other;
    AddSigned(copy, otherNegative);
    return;
  }
  if (isNegative == otherNegative) {
    limbs = AddMag(limbs, other.limbs);
  } else if (CompareMag(limbs, other.limbs) >= 0) {
    limbs = SubMag(limbs, other.limbs);
  } else {
    limbs = SubMag(other.limbs, limbs);
    isNegative = otherNegative;
  }
  trim();
}

BigInt &BigInt::operator+=(const BigInt &other) {
  AddSigned(other, other.isNegative);
  return *this;
}

BigInt &BigInt::operator-=(const BigInt &other) {
  AddSigned(other, !other.isNegative);
  return *this;
}

BigInt &BigInt::operator*=(const BigInt &other) {
  isNegative = isNegative != other.isNegative;
  limbs = MulMag(limbs, other.limbs);
  trim();
  return *this;
}

BigInt operator+(BigInt leftHandSide, const BigInt &rightHandSide) {
  leftHandSide += rightHandSide;
  return leftHandSide;
}
BigInt operator-(BigInt leftHandSide, const BigInt &rightHandSide) {
  leftHandSide -= rightHandSide;
  return leftHandSide;
}
BigInt operator*(BigInt leftHandSide, const BigInt &rightHandSide) {
  leftHandSide *= rightHandSide;
  return leftHandSide;
}

BigInt BigInt::operator-() const {
  BigInt r = *this;
  r.isNegative = !isNegative;
  r.trim(); // keeps zero non-negative
  return r;
}

void BigInt::DivMod(const BigInt &a, const BigInt &b, BigInt &quotient,
                    BigInt &remainder) {
  if (b.IsZero())
    throw std::domain_error("BigInt: division by zero");
  Mag qm, rm;
  DivModMag(a.limbs, b.limbs, qm, rm);

  bool qNeg = a.isNegative != b.isNegative;
  if (a.isNegative && !(rm.size() == 1 && rm[0] == 0)) {
    // truncated -> Euclidean: remainder = |b| - rm, magnitude of q grows by 1
    rm = SubMag(b.limbs, rm);
    qm = AddMag(qm, Mag{1});
  }
  // assign last: a or b may alias the outputs
  quotient.limbs = std::move(qm);
  quotient.isNegative = qNeg;
  quotient.trim();
  remainder.limbs = std::move(rm);
  remainder.isNegative = false;
  remainder.trim();
}

BigInt BigInt::operator/(const BigInt &divisor) const {
  BigInt q, r;
  DivMod(*this, divisor, q, r);
  return q;
}

BigInt BigInt::operator%(const BigInt &mod) const {
  BigInt q, r;
  DivMod(*this, mod, q, r);
  return r;
}

BigInt &BigInt::operator/=(const BigInt &divisor) {
  return *this = *this / divisor;
}
BigInt &BigInt::operator%=(const BigInt &mod) { return *this = *this % mod; }

BigInt BigInt::operator<<(size_t bits) const {
  BigInt r;
  r.limbs = ShiftLeftMag(limbs, bits);
  r.isNegative = isNegative;
  r.trim();
  return r;
}

BigInt BigInt::operator>>(size_t bits) const {
  BigInt r;
  r.limbs = ShiftRightMag(limbs, bits);
  r.isNegative = isNegative;
  r.trim();
  return r;
}

size_t BigInt::BitLength() const {
  if (IsZero())
    return 0;
  return limbs.size() * 32 - static_cast<size_t>(std::countl_zero(limbs.back()));
}

bool BigInt::TestBit(size_t index) const {
  size_t limb = index / 32;
  if (limb >= limbs.size())
    return false;
  return (limbs[limb] >> (index % 32)) & 1;
}

bool BigInt::IsOdd() const { return limbs[0] & 1; }

bool BigInt::IsZero() const { return limbs.size() == 1 && limbs[0] == 0; }

bool BigInt::operator==(const BigInt &other) const {
  return isNegative == other.isNegative && limbs == other.limbs;
}

std::string BigInt::ToHexString() const {
  static const char digits[] = "0123456789abcdef";
  std::string out;
  for (size_t i = limbs.size(); i-- > 0;) {
    for (int shift = 28; shift >= 0; shift -= 4) {
      char c = digits[(limbs[i] >> shift) & 0xF];
      if (out.empty() && c == '0')
        continue; // skip leading zeros
      out.push_back(c);
    }
  }
  if (out.empty())
    return "0";
  return isNegative ? "-" + out : out;
}

std::ostream &operator<<(std::ostream &os, const BigInt &num) {
  return os << num.ToHexString();
}
