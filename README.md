# RSAEncryptionFromScratch

A from-scratch implementation of the RSA (Rivest-Shamir-Adleman) cryptosystem in C++20, written **without any external libraries**. Everything RSA needs is built here: an arbitrary-precision integer type, primality testing, prime and key generation, and encryption/decryption.

> **Educational only.** This is textbook RSA with no padding. It is deterministic and malleable, and must not be used to protect real data.

## Features

- **`BigInt`**: signed arbitrary-precision integers (32-bit limbs, sign-magnitude)
  - `+ - *`, Euclidean division and modulo (Knuth Algorithm D), shifts, comparison
  - `ModExp` (square-and-multiply), `Gcd`, `ModInverse` (extended Euclid)
  - hex string and big-endian byte conversion
- **Primes**: uniform random sampling, trial division plus Miller-Rabin primality test, random prime generation
- **RSA**: key generation (e = 65537, d computed from Carmichael's lambda), encryption and decryption of integers, with text handled via byte conversion

## Build and run

Requires CMake 3.16+ and a C++20 compiler (developed with g++).

```
cmake -B build && cmake --build build
./build/rsa                                   # generate a 1024-bit key and round-trip a message
ctest --test-dir build --output-on-failure    # run the tests
./build/tests ModInverse                      # run only tests whose name contains "ModInverse"
```

For a faster demo and key generation, configure with `-DCMAKE_BUILD_TYPE=Release`. To catch memory errors, configure with `-DCMAKE_CXX_FLAGS="-fsanitize=address,undefined"`.

## Usage

```cpp
#include "BigInt.hpp"
#include "Rsa.hpp"

RsaKeyPair keys = GenerateKeyPair(1024);          // 1024-bit modulus

std::string text = "hello, RSA";
BigInt m = BigInt::FromBytes({text.begin(), text.end()});
BigInt c = Encrypt(keys.publicKey, m);            // m^e mod n
BigInt back = Decrypt(keys.privateKey, c);        // c^d mod n

std::vector<uint8_t> bytes = back.ToBytes();      // original message bytes
```

`BigInt` can also be used on its own:

```cpp
BigInt a("ffffffffffffffff");                     // hex input
BigInt b = (a * a + BigInt(1)) % BigInt("123456789");
BigInt inv = BigInt::ModInverse(BigInt(17), BigInt(3120));   // 2753
std::cout << b << "\n";                           // hex output
```

## Project layout

| Path | Contents |
| --- | --- |
| `include/BigInt.hpp`, `src/BigInt.cpp` | arbitrary-precision integer and number theory |
| `include/Prime.hpp`, `src/Prime.cpp` | random sampling, Miller-Rabin, prime generation |
| `include/Rsa.hpp`, `src/Rsa.cpp` | key generation, encrypt, decrypt |
| `src/main.cpp` | demo program |
| `tests/` | in-repo test harness and tests (no test framework dependency) |

## Design notes

- `BigInt` stores a little-endian `std::vector<uint32_t>` of limbs plus a sign flag. Zero is always `{0}` and never negative.
- `%` is Euclidean: the result is always in `[0, |m|)`, so `-7 % 2 == 1`. Dividing by zero, a negative `ModExp` exponent, or a non-invertible `ModInverse` throws.
- The number type is tested against `__int128` on random inputs and against algebraic identities (such as `q*b + r == a`) on numbers hundreds of bits long. Primality testing is checked against Carmichael numbers, a strong pseudoprime, and Mersenne primes.
- Randomness comes from `std::random_device`, which uses the operating system's entropy source on Linux.

## Limitations

- No padding: identical messages produce identical ciphertexts, and leading zero bytes of a message are lost on a round trip.
- Arithmetic is schoolbook and not constant-time, so it is not hardened against timing attacks.
- Decryption does not use the Chinese Remainder Theorem optimization.

## Roadmap

- OAEP padding (requires SHA-256 implemented from scratch)
- CRT-based decryption
