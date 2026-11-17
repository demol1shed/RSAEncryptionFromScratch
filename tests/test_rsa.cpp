#include "BigInt.hpp"
#include "Prime.hpp"
#include "Rsa.hpp"
#include "test.hpp"
#include <stdexcept>

TEST(BytesRoundTrip) {
  CHECK(BigInt::FromBytes({}).IsZero());
  CHECK(BigInt::FromBytes({0, 0, 0}).IsZero());
  CHECK(BigInt::FromBytes({0x01, 0x00}) == BigInt(256));
  CHECK(BigInt::FromBytes({0x00, 0xff}) == BigInt(255));
  CHECK(BigInt::FromBytes({1, 2, 3, 4, 5}).ToHexString() == "102030405");
  CHECK(BigInt(0).ToBytes().empty());
  CHECK((BigInt("102030405").ToBytes() ==
         std::vector<uint8_t>{1, 2, 3, 4, 5}));
  CHECK((BigInt(255).ToBytes(3) == std::vector<uint8_t>{0, 0, 255}));
  CHECK((BigInt(0).ToBytes(2) == std::vector<uint8_t>{0, 0}));
  CHECK_THROWS(BigInt(256).ToBytes(1), std::length_error);
  CHECK_THROWS(BigInt(-1).ToBytes(), std::domain_error);
  for (size_t len = 1; len < 40; ++len) {
    std::vector<uint8_t> v(len);
    for (size_t i = 0; i < len; ++i)
      v[i] = static_cast<uint8_t>(i * 37 + 11); // first byte nonzero
    CHECK(BigInt::FromBytes(v).ToBytes() == v);
  }
}

TEST(RandomBelowRange) {
  CHECK_THROWS(RandomBelow(BigInt(0)), std::domain_error);
  CHECK(RandomBelow(BigInt(1)).IsZero());
  bool seen[5] = {};
  for (int i = 0; i < 200; ++i) {
    BigInt r = RandomBelow(BigInt(5));
    CHECK(r >= BigInt(0) && r < BigInt(5));
    seen[r.ToBytes().empty() ? 0 : r.ToBytes()[0]] = true;
  }
  for (bool s : seen)
    CHECK(s);
  BigInt big("123456789abcdef0123456789abcdef");
  for (int i = 0; i < 50; ++i)
    CHECK(RandomBelow(big) < big);
}

TEST(PrimalityKnown) {
  for (int p : {2, 3, 5, 7, 997, 1009, 7919, 104729, 1000003})
    CHECK(IsProbablePrime(BigInt(p)));
  for (int c : {-7, 0, 1, 4, 9, 1001, 1000001, 999999})
    CHECK(!IsProbablePrime(BigInt(c)));
  // Carmichael numbers and a strong pseudoprime to base 2
  for (int64_t c : {561LL, 41041LL, 825265LL, 321197185LL, 2047LL, 3215031751LL})
    CHECK(!IsProbablePrime(BigInt(c)));
  CHECK(IsProbablePrime((BigInt(1) << 89) - BigInt(1)));
  CHECK(IsProbablePrime((BigInt(1) << 127) - BigInt(1)));
  CHECK(!IsProbablePrime((BigInt(1) << 128) - BigInt(1)));
  CHECK(!IsProbablePrime((BigInt(1) << 67) - BigInt(1))); // composite Mersenne
  BigInt p = (BigInt(1) << 89) - BigInt(1), q = (BigInt(1) << 127) - BigInt(1);
  CHECK(!IsProbablePrime(p * q));
}

TEST(GeneratePrimeProperties) {
  CHECK_THROWS(GeneratePrime(7), std::invalid_argument);
  for (size_t bits : {8, 9, 16, 31, 32, 33, 64, 128}) {
    BigInt p = GeneratePrime(bits);
    CHECK(p.BitLength() == bits);
    CHECK(p.TestBit(bits - 2));
    CHECK(IsProbablePrime(p));
  }
}

TEST(RsaRoundTrip) {
  CHECK_THROWS(GenerateKeyPair(16), std::invalid_argument);
  for (size_t bits : {32, 33, 64, 256}) {
    RsaKeyPair k = GenerateKeyPair(bits);
    CHECK(k.publicKey.n.BitLength() == bits);
    CHECK(k.publicKey.n == k.privateKey.n);
    for (int i = 0; i < 10; ++i) {
      BigInt m = RandomBelow(k.publicKey.n);
      CHECK(Decrypt(k.privateKey, Encrypt(k.publicKey, m)) == m);
    }
    CHECK(Decrypt(k.privateKey, Encrypt(k.publicKey, BigInt(0))).IsZero());
    CHECK(Decrypt(k.privateKey, Encrypt(k.publicKey, BigInt(1))) == BigInt(1));
    BigInt top = k.publicKey.n - BigInt(1);
    CHECK(Decrypt(k.privateKey, Encrypt(k.publicKey, top)) == top);
  }
}

TEST(RsaRejectsOutOfRange) {
  RsaKeyPair k = GenerateKeyPair(64);
  CHECK_THROWS(Encrypt(k.publicKey, k.publicKey.n), std::invalid_argument);
  CHECK_THROWS(Encrypt(k.publicKey, BigInt(-1)), std::invalid_argument);
  CHECK_THROWS(Decrypt(k.privateKey, k.privateKey.n), std::invalid_argument);
}

TEST(RsaTextRoundTrip) {
  RsaKeyPair k = GenerateKeyPair(512);
  std::string text = "attack at dawn";
  BigInt m = BigInt::FromBytes(std::vector<uint8_t>(text.begin(), text.end()));
  auto out = Decrypt(k.privateKey, Encrypt(k.publicKey, m)).ToBytes();
  CHECK(std::string(out.begin(), out.end()) == text);
}
