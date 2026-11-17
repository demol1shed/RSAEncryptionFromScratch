#include "BigInt.hpp"
#include <algorithm>
#include <cstdint>
#include <stdexcept>
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
