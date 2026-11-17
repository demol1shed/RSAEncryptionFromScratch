#include "BigInt.hpp"
#include "test.hpp"
#include <cstdint>
#include <limits>
#include <stdexcept>

TEST(DefaultIsZero) {
  BigInt z;
  CHECK(z.IsZero());
  CHECK(z.ToHexString() == "0");
  CHECK(z == BigInt(0));
}

TEST(Int64Ctor) {
  CHECK(BigInt(255).ToHexString() == "ff");
  CHECK(BigInt(-255).ToHexString() == "-ff");
  CHECK(BigInt(int64_t{1} << 32).ToHexString() == "100000000");
  CHECK(BigInt(std::numeric_limits<int64_t>::max()).ToHexString() ==
        "7fffffffffffffff");
  CHECK(BigInt(std::numeric_limits<int64_t>::min()).ToHexString() ==
        "-8000000000000000");
}

TEST(HexCtorBasics) {
  CHECK(BigInt("0").IsZero());
  CHECK(BigInt("-0").IsZero());
  CHECK(BigInt("-0") == BigInt(0)); // no negative zero
  CHECK(BigInt("000000000000000000ff").ToHexString() == "ff");
  CHECK(BigInt("FF") == BigInt("ff"));
  CHECK(BigInt("-1f") == BigInt(-31));
}

TEST(HexCtorMultiLimb) {
  // lengths around the 8-digit limb boundary
  for (std::string s : {"1", "12345678", "123456789", "1234567812345678",
                        "123456789abcdef0123456789abcdef01"}) {
    CHECK(BigInt(s).ToHexString() == s);
    CHECK(BigInt("-" + s).ToHexString() == "-" + s);
  }
  CHECK(BigInt("ffffffffffffffff") == BigInt("0ffffffffffffffff"));
}

TEST(HexCtorRejectsBadInput) {
  CHECK_THROWS(BigInt(std::string("")), std::invalid_argument);
  CHECK_THROWS(BigInt(std::string("-")), std::invalid_argument);
  CHECK_THROWS(BigInt(std::string("0x10")), std::invalid_argument);
  CHECK_THROWS(BigInt(std::string("12-3")), std::invalid_argument);
  CHECK_THROWS(BigInt(std::string("+5")), std::invalid_argument);
  CHECK_THROWS(BigInt(std::string(" 5")), std::invalid_argument);
  CHECK_THROWS(BigInt(std::string("xyz")), std::invalid_argument);
  CHECK_THROWS(BigInt(std::string("--1")), std::invalid_argument);
}

// ---- arithmetic ----
#include <random>

__extension__ typedef __int128 i128;
__extension__ typedef unsigned __int128 u128;

static std::string Hex128(i128 v) {
  bool neg = v < 0;
  u128 m = neg ? -static_cast<u128>(v)
                            : static_cast<u128>(v);
  if (m == 0)
    return "0";
  std::string s;
  while (m) {
    s.insert(s.begin(), "0123456789abcdef"[static_cast<int>(m & 0xF)]);
    m >>= 4;
  }
  return neg ? "-" + s : s;
}
static BigInt B(i128 v) { return BigInt(Hex128(v)); }

static std::string RandomHex(std::mt19937_64 &rng, size_t digits) {
  std::string s;
  for (size_t i = 0; i < digits; ++i)
    s.push_back("0123456789abcdef"[rng() % 16]);
  if (rng() % 2)
    s = "-" + s;
  return s;
}

TEST(AddSubMulMatchInt128) {
  std::mt19937_64 rng(1);
  for (int i = 0; i < 2000; ++i) {
    int64_t x = static_cast<int64_t>(rng()) >> (rng() % 63);
    int64_t y = static_cast<int64_t>(rng()) >> (rng() % 63);
    i128 a = x, b = y;
    CHECK(BigInt(x) + BigInt(y) == B(a + b));
    CHECK(BigInt(x) - BigInt(y) == B(a - b));
    CHECK(BigInt(x) * BigInt(y) == B(a * b));
  }
}

TEST(DivModMatchInt128) {
  std::mt19937_64 rng(2);
  for (int i = 0; i < 2000; ++i) {
    i128 a = (static_cast<i128>(static_cast<int64_t>(rng())) << 32) ^
             static_cast<int64_t>(rng());
    a >>= rng() % 90;
    i128 b = static_cast<int64_t>(rng()) >> (rng() % 63);
    if (b == 0)
      continue;
    // Euclidean reference
    i128 q = a / b, r = a % b;
    if (r < 0) {
      r += b < 0 ? -b : b;
      q = (a - r) / b;
    }
    BigInt bq, br;
    BigInt::DivMod(B(a), B(b), bq, br);
    CHECK(bq == B(q));
    CHECK(br == B(r));
    CHECK(B(a) % B(b) == B(r));
  }
}

TEST(DivModIdentityLarge) {
  std::mt19937_64 rng(3);
  for (int i = 0; i < 500; ++i) {
    BigInt a(RandomHex(rng, 1 + rng() % 120));
    BigInt b(RandomHex(rng, 1 + rng() % 70));
    if (b.IsZero())
      continue;
    BigInt q, r;
    BigInt::DivMod(a, b, q, r);
    CHECK(q * b + r == a);
    CHECK(r >= BigInt(0));
    CHECK(r < (b < BigInt(0) ? -b : b));
  }
}

TEST(DivModAddBackCase) {
  // classic Knuth D add-back trigger
  BigInt u("800000000000000000000003");
  BigInt v("200000000000000000000001");
  BigInt q, r;
  BigInt::DivMod(u, v, q, r);
  CHECK(q * v + r == u);
  CHECK(r < v);
  BigInt u2("7fffffff800000010000000000000000");
  BigInt v2("800000008000000200000005");
  BigInt::DivMod(u2, v2, q, r);
  CHECK(q * v2 + r == u2);
  CHECK(r < v2);
}

TEST(DivModSigns) {
  CHECK(BigInt(-7) % BigInt(2) == BigInt(1));
  CHECK(BigInt(-7) / BigInt(2) == BigInt(-4));
  CHECK(BigInt(7) % BigInt(-2) == BigInt(1));
  CHECK(BigInt(7) / BigInt(-2) == BigInt(-3));
  CHECK(BigInt(-7) % BigInt(-2) == BigInt(1));
  CHECK(BigInt(-7) / BigInt(-2) == BigInt(4));
  CHECK(BigInt(-6) % BigInt(3) == BigInt(0));
  CHECK_THROWS(BigInt(1) / BigInt(0), std::domain_error);
  CHECK_THROWS(BigInt(1) % BigInt(0), std::domain_error);
}

TEST(AliasingAndZero) {
  BigInt a("123456789abcdef0123");
  BigInt b = a;
  a += a;
  CHECK(a == b * BigInt(2));
  a -= a;
  CHECK(a.IsZero());
  CHECK(!(-BigInt(0) < BigInt(0)));
  CHECK((BigInt(5) - BigInt(5)).ToHexString() == "0");
  CHECK((BigInt(-5) * BigInt(0)).ToHexString() == "0");
  BigInt c("ffffffffffffffff");
  c *= c;
  CHECK(c.ToHexString() == "fffffffffffffffe0000000000000001");
  CHECK(BigInt("ffffffff") + BigInt(1) == BigInt("100000000"));
  CHECK(BigInt("100000000") - BigInt(1) == BigInt("ffffffff"));
}

TEST(Ordering) {
  CHECK(BigInt(-5) < BigInt(3));
  CHECK(BigInt(-5) < BigInt(-3));
  CHECK(BigInt("100000000") > BigInt("ffffffff"));
  CHECK(BigInt(0) == -BigInt(0));
  CHECK(BigInt(2) >= BigInt(2));
}

TEST(ShiftsAndBits) {
  CHECK((BigInt(1) << 100).ToHexString() == "10000000000000000000000000");
  CHECK(((BigInt(1) << 100) >> 100) == BigInt(1));
  CHECK((BigInt("ff") >> 4) == BigInt("f"));
  CHECK((BigInt("ff") >> 100).IsZero());
  CHECK((BigInt("123456789abcdef") << 37) >> 37 == BigInt("123456789abcdef"));
  CHECK(BigInt(0).BitLength() == 0);
  CHECK(BigInt(1).BitLength() == 1);
  CHECK(BigInt("100000000").BitLength() == 33);
  CHECK(BigInt(5).TestBit(0) && !BigInt(5).TestBit(1) && BigInt(5).TestBit(2));
  CHECK(!BigInt(5).TestBit(500));
  CHECK(BigInt(7).IsOdd() && !BigInt(8).IsOdd() && !BigInt(0).IsOdd());
}
