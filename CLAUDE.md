# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Textbook RSA in C++20 written from scratch, with no external libraries (tests included). Educational only: no padding, so it is not secure. Layers, bottom to top: `BigInt` -> primes (`Prime`) -> RSA (`Rsa`).

## Build / test / run

```
cmake -B build && cmake --build build
ctest --test-dir build --output-on-failure   # all tests
./build/tests DivMod                         # only tests whose name contains "DivMod"
./build/rsa                                  # 1024-bit keygen + encrypt/decrypt demo
```

For memory-error checking configure with `-DCMAKE_CXX_FLAGS="-fsanitize=address,undefined"` (tests are slow-ish under sanitizers, ~1 s; the demo is fast only in a Release build). The compiler flags are `-Wall -Wextra -Wpedantic` and the tree builds warning-free; keep it that way.

Tests use a tiny in-repo harness (`tests/test.hpp`: `TEST(name)`, `CHECK`, `CHECK_THROWS`; tests self-register, so a new test only needs to live in a file listed in the `tests` target in `CMakeLists.txt`). Large-number results are cross-checked against `__int128` and algebraic identities (e.g. `q*b + r == a`).

## Architecture

- `include/BigInt.hpp` / `src/BigInt.cpp`: arbitrary-precision signed integer. Sign-magnitude: `std::vector<uint32_t> limbs` (little-endian) plus `isNegative`.
  - **Invariants** (restored by `trim()`): `limbs` is never empty, has no high zero limbs (zero is `{0}`), and zero is never negative. Any operation that can shrink the magnitude must end with `trim()`.
  - Magnitude arithmetic (`AddMag`, `SubMag`, `MulMag`, `DivModMag` = Knuth Algorithm D, shifts) lives in an anonymous namespace in `BigInt.cpp`; the public signed operators are built on it.
  - **Division is Euclidean:** `DivMod`/`%` always give `0 <= r < |b|` (so `-7 % 2 == 1`, `-7 / 2 == -4`); only a zero divisor throws. `>>` truncates the magnitude toward zero.
  - `ModInverse` throws `std::domain_error` if `m <= 0` or not invertible; `ModExp` throws on zero modulus or negative exponent.
  - The string constructor and `ToHexString` use **hex** (optional leading `-`); malformed input throws `std::invalid_argument`. `FromBytes`/`ToBytes` are big-endian unsigned.
- `include/Prime.hpp` / `src/Prime.cpp`: `RandomBelow`, `IsProbablePrime` (trial division then Miller-Rabin), `GeneratePrime(bits)` (forces top two bits so p*q has exactly 2*bits bits). Randomness is `std::random_device`.
- `include/Rsa.hpp` / `src/Rsa.cpp`: `GenerateKeyPair` (e = 65537, d = e^-1 mod lcm(p-1, q-1)), `Encrypt`/`Decrypt` on `BigInt` (inputs must be in `[0, n)`). Text goes through `BigInt::FromBytes`/`ToBytes`; without padding, leading zero bytes of a message are lost.
- `src/main.cpp`: demo only. All sources except `main.cpp` and the tests compile into the `bigint` static library.

## Not done

OAEP padding (would need SHA-256 from scratch) is an optional later phase; CRT decryption is not implemented.
