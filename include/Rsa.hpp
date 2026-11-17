#pragma once
#include "BigInt.hpp"
#include <cstddef>

// Textbook RSA (no padding). Educational only: deterministic and malleable,
// NOT secure for real use.

struct RsaPublicKey {
  BigInt n; // modulus
  BigInt e; // public exponent
};

struct RsaPrivateKey {
  BigInt n; // modulus
  BigInt d; // private exponent
};

struct RsaKeyPair {
  RsaPublicKey publicKey;
  RsaPrivateKey privateKey;
};

// e = 65537. The modulus has exactly `modulusBits` bits. Requires
// modulusBits >= 32, otherwise throws std::invalid_argument.
RsaKeyPair GenerateKeyPair(size_t modulusBits);

// m must satisfy 0 <= m < n, otherwise std::invalid_argument is thrown.
BigInt Encrypt(const RsaPublicKey &key, const BigInt &message);
// c must satisfy 0 <= c < n, otherwise std::invalid_argument is thrown.
BigInt Decrypt(const RsaPrivateKey &key, const BigInt &ciphertext);
