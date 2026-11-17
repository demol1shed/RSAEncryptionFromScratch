#pragma once
#include <compare>
#include <cstddef>
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
  // *this += (otherNegative ? -other : other); safe when &other == this
  void AddSigned(const BigInt &other, bool otherNegative);

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

  BigInt operator-() const;

  // Euclidean division: a = q * b + r with 0 <= r < |b|. Throws
  // std::domain_error if b is zero.
  static void DivMod(const BigInt &a, const BigInt &b, BigInt &quotient,
                     BigInt &remainder);
  BigInt operator/(const BigInt &divisor) const;
  // result is always in [0, |mod|); throws std::domain_error if mod is zero
  BigInt operator%(const BigInt &mod) const;
  BigInt &operator/=(const BigInt &divisor);
  BigInt &operator%=(const BigInt &mod);

  // shift the magnitude (sign is kept; >> truncates toward zero)
  BigInt operator<<(size_t bits) const;
  BigInt operator>>(size_t bits) const;

  size_t BitLength() const;         // 0 for zero
  bool TestBit(size_t index) const; // bit of the magnitude
  bool IsOdd() const;

  // a^e (mod m), result in [0, |m|). Throws std::domain_error if m is zero
  // or e is negative.
  static BigInt ModExp(const BigInt &base, const BigInt &exp,
                       const BigInt &mod);

  // non-negative; Gcd(0, 0) == 0
  static BigInt Gcd(BigInt a, BigInt b);
  // x with a*x == 1 (mod m), x in [0, m). Extended Euclid. Throws
  // std::domain_error if m <= 0 or gcd(a, m) != 1.
  static BigInt ModInverse(const BigInt &a, const BigInt &m);

  bool IsZero() const;

  // debug and output
  std::string ToHexString() const;
  friend std::ostream &operator<<(std::ostream &os, const BigInt &num);
};
