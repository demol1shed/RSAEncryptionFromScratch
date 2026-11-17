#include "Prime.hpp"
#include <cstdint>
#include <random>
#include <stdexcept>
#include <vector>

namespace {
std::vector<uint8_t> RandomBytes(size_t count) {
  static std::random_device rd;
  std::vector<uint8_t> out(count);
  for (size_t i = 0; i < count; i += 4) {
    uint32_t word = rd();
    for (size_t j = 0; j < 4 && i + j < count; ++j)
      out[i + j] = static_cast<uint8_t>(word >> (8 * j));
  }
  return out;
}

// bit `index` (0 = least significant) of a big-endian byte string
void SetBit(std::vector<uint8_t> &bytes, size_t index) {
  bytes[bytes.size() - 1 - index / 8] |= static_cast<uint8_t>(1u << (index % 8));
}

// clear everything at or above bit `bits`
void MaskToBits(std::vector<uint8_t> &bytes, size_t bits) {
  size_t extra = bytes.size() * 8 - bits;
  bytes[0] &= static_cast<uint8_t>(0xFFu >> extra);
}

const std::vector<uint32_t> &SmallPrimes() {
  static const std::vector<uint32_t> primes = [] {
    std::vector<uint32_t> p;
    for (uint32_t n = 2; n < 1000; ++n) {
      bool isPrime = true;
      for (uint32_t d : p)
        if (d * d <= n && n % d == 0) {
          isPrime = false;
          break;
        }
      if (isPrime)
        p.push_back(n);
    }
    return p;
  }();
  return primes;
}
} // namespace

BigInt RandomBelow(const BigInt &upper) {
  if (upper <= BigInt(0))
    throw std::domain_error("RandomBelow: upper bound must be positive");
  size_t bits = upper.BitLength();
  size_t nbytes = (bits + 7) / 8;
  // rejection sampling keeps the distribution uniform
  while (true) {
    auto bytes = RandomBytes(nbytes);
    MaskToBits(bytes, bits);
    BigInt candidate = BigInt::FromBytes(bytes);
    if (candidate < upper)
      return candidate;
  }
}

bool IsProbablePrime(const BigInt &n, int rounds) {
  if (n < BigInt(2))
    return false;
  for (uint32_t p : SmallPrimes()) {
    BigInt bp(static_cast<int64_t>(p));
    if (n == bp)
      return true;
    if ((n % bp).IsZero())
      return false;
  }
  if (n < BigInt(1000 * 1000)) // no factor below 1000 and n < 1000^2
    return true;

  // n - 1 = d * 2^s with d odd
  const BigInt one(1);
  const BigInt nMinus1 = n - one;
  size_t s = 0;
  while (!nMinus1.TestBit(s))
    ++s;
  const BigInt d = nMinus1 >> s;

  for (int round = 0; round < rounds; ++round) {
    BigInt a = RandomBelow(n - BigInt(3)) + BigInt(2); // [2, n-2]
    BigInt x = BigInt::ModExp(a, d, n);
    if (x == one || x == nMinus1)
      continue;
    bool witnessComposite = true;
    for (size_t r = 1; r < s; ++r) {
      x = (x * x) % n;
      if (x == nMinus1) {
        witnessComposite = false;
        break;
      }
    }
    if (witnessComposite)
      return false;
  }
  return true;
}

BigInt GeneratePrime(size_t bits) {
  if (bits < 8)
    throw std::invalid_argument("GeneratePrime: need at least 8 bits");
  size_t nbytes = (bits + 7) / 8;
  while (true) {
    auto bytes = RandomBytes(nbytes);
    MaskToBits(bytes, bits);
    SetBit(bytes, bits - 1);
    SetBit(bytes, bits - 2);
    SetBit(bytes, 0);
    BigInt candidate = BigInt::FromBytes(bytes);
    if (IsProbablePrime(candidate))
      return candidate;
  }
}
