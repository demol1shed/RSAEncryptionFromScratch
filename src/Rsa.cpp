#include "Rsa.hpp"
#include "Prime.hpp"
#include <stdexcept>

RsaKeyPair GenerateKeyPair(size_t modulusBits) {
  if (modulusBits < 32)
    throw std::invalid_argument("GenerateKeyPair: need at least 32 bits");

  const BigInt e(65537);
  const BigInt one(1);
  while (true) {
    BigInt p = GeneratePrime(modulusBits / 2);
    BigInt q = GeneratePrime(modulusBits - modulusBits / 2);
    if (p == q)
      continue;
    BigInt n = p * q;
    BigInt pm1 = p - one, qm1 = q - one;
    // Carmichael's lambda(n) = lcm(p-1, q-1)
    BigInt lambda = (pm1 * qm1) / BigInt::Gcd(pm1, qm1);
    if (BigInt::Gcd(e, lambda) != one)
      continue;
    BigInt d = BigInt::ModInverse(e, lambda);
    return {{n, e}, {n, d}};
  }
}

namespace {
void CheckRange(const BigInt &value, const BigInt &n, const char *what) {
  if (value < BigInt(0) || value >= n)
    throw std::invalid_argument(std::string(what) + " must be in [0, n)");
}
} // namespace

BigInt Encrypt(const RsaPublicKey &key, const BigInt &message) {
  CheckRange(message, key.n, "message");
  return BigInt::ModExp(message, key.e, key.n);
}

BigInt Decrypt(const RsaPrivateKey &key, const BigInt &ciphertext) {
  CheckRange(ciphertext, key.n, "ciphertext");
  return BigInt::ModExp(ciphertext, key.d, key.n);
}
