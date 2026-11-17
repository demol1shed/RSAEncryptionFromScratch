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
