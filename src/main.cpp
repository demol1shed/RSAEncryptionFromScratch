#include "BigInt.hpp"
#include "Rsa.hpp"
#include <iostream>
#include <string>
#include <vector>

// Textbook RSA demo. Educational only: not secure.
int main() {
  const size_t bits = 1024;
  std::cout << "generating " << bits << "-bit key pair...\n";
  RsaKeyPair keys = GenerateKeyPair(bits);
  std::cout << "n = " << keys.publicKey.n << "\ne = " << keys.publicKey.e
            << "\n";

  std::string text = "hello, RSA";
  BigInt m = BigInt::FromBytes(std::vector<uint8_t>(text.begin(), text.end()));
  BigInt c = Encrypt(keys.publicKey, m);
  std::cout << "ciphertext = " << c << "\n";

  std::vector<uint8_t> out = Decrypt(keys.privateKey, c).ToBytes();
  std::cout << "decrypted  = " << std::string(out.begin(), out.end()) << "\n";
  return std::string(out.begin(), out.end()) == text ? 0 : 1;
}
