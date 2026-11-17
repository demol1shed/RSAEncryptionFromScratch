# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

RSA cryptosystem in C++20 written from scratch, with no external libraries (see README). The foundation is a custom arbitrary-precision `BigInt` class; RSA key generation, encryption and decryption are meant to be built on top of it.

## Build / run

There is no build system (no CMake/Makefile) and no test framework yet. `compile_flags.txt` (`-std=c++20 -Iinclude`) exists for clangd. Build manually:

```
g++ -std=c++20 -Wall -Wextra -Iinclude src/*.cpp -o rsa && ./rsa
```

Build outputs (`*.o`, `*.out`, `*.exe`, ...) are gitignored; an executable named `rsa` is not, so don't commit it.

## Architecture

- `include/BigInt.hpp` is the public interface; `src/BigInt.cpp` the implementation; `src/main.cpp` is currently a placeholder that prints "compile".
- **Representation:** `std::vector<uint32_t> limbs` in little-endian order (limb 0 is least significant) plus a separate `isNegative` sign flag (sign-magnitude, not two's complement).
- **Invariants:** `trim()` strips leading (high) zero limbs but keeps at least one limb, and clears the sign on zero (no negative zero). Call `trim()` after any operation that can shrink the magnitude. Constructors from `int64_t` and from a value of 0 store a single `{0}` limb, but the defaulted `BigInt()` leaves `limbs` empty, so code must not assume `limbs` is non-empty unless the default constructor is changed.
- **Construction:** both constructors are `explicit`. The string constructor takes **hex** (optional leading `-`), parsed in 8-hex-digit chunks from the right so each chunk maps to one 32-bit limb. `ToHexString()` is the intended inverse.

## State of work (branch `feature/bigIntImplementation`)

Only the two constructors and `trim()` are implemented. Everything else declared in `BigInt.hpp` (`<=>`, `==`, `+=`, `-=`, `*=`, the free `+ - *`, `%`, `ModExp`, `gcd`, `ModInverse`, `ToHexString`, `operator<<`) is declared but has no definition, so using any of them fails at link time. Implement them in `src/BigInt.cpp`. `ModExp`, `gcd` and `ModInverse` (extended Euclid) are the primitives RSA needs.
