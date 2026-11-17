#pragma once
#include "BigInt.hpp"
#include <cstddef>

// Randomness comes from std::random_device (the OS entropy source on Linux).

// Uniform random value in [0, upper). Throws std::domain_error if upper <= 0.
BigInt RandomBelow(const BigInt &upper);

// Miller-Rabin with random bases, after trial division by small primes.
// Error probability for a composite is at most 4^-rounds.
bool IsProbablePrime(const BigInt &n, int rounds = 40);

// Random prime of exactly `bits` bits whose top two bits are set (so the
// product of two such primes of b bits has exactly 2b bits). Requires
// bits >= 8, otherwise throws std::invalid_argument.
BigInt GeneratePrime(size_t bits);
