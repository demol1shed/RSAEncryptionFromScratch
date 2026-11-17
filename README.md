# RSAEncryptionFromScratch
A simple implementation of the RSA (Rivest-Shamir-Adleman) cryptosystem in C++20, made without any use of external libraries. It includes its own arbitrary-precision integer (`BigInt`), Miller-Rabin primality testing, key generation, and encryption/decryption.

> **Educational only.** This is textbook RSA with no padding and is not secure for real use.

## Build and run
```
cmake -B build && cmake --build build
./build/rsa                                   # generate a 1024-bit key and round-trip a message
ctest --test-dir build --output-on-failure    # run the tests
```
Requires CMake 3.16+ and a C++20 compiler (tested with g++).
