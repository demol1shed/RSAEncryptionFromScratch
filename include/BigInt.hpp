#pragma once
#include <compare>
#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

class BigInt {
private:
  // Magnitude, little-endian. Invariant: never empty, no high zero limbs
  // (zero is {0}), and zero is never negative.
  std::vector<uint32_t> limbs{0};
  bool isNegative{false};

  // trim leading zeros
  void trim();

public:
  BigInt() = default;
  // avoid compiler automatic type conversion
  explicit BigInt(int64_t val);
  // hex digits with optional leading '-'; throws std::invalid_argument on
  // empty or malformed input
  explicit BigInt(const std::string &hexStr);

  // operator overloading for bigint types
  std::strong_ordering operator<=>(const BigInt &other) const;
  bool operator==(const BigInt &other) const;

  BigInt &operator+=(const BigInt &other);
  BigInt &operator-=(const BigInt &other);
  BigInt &operator*=(const BigInt &other);

  friend BigInt operator+(BigInt leftHandSide, const BigInt &rightHandSide);
  friend BigInt operator-(BigInt leftHandSide, const BigInt &rightHandSide);
  friend BigInt operator*(BigInt leftHandSide, const BigInt &rightHandSide);

  BigInt operator%(const BigInt &mod) const;

  // a^e (mod m)
  static BigInt ModExp(const BigInt &base, const BigInt &exp,
                       const BigInt &mod);

  static BigInt Gcd(BigInt a, BigInt b);
  // extended euclidean
  static BigInt ModInverse(const BigInt &a, const BigInt &m);

  bool IsZero() const;

  // debug and output
  std::string ToHexString() const;
  friend std::ostream &operator<<(std::ostream &os, const BigInt &num);
};
