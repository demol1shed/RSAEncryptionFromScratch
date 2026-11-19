# CLAUDE.md

Guidance for Claude Code (claude.ai/code) in this repository.

Architecture, invariants and architectural constraints live in `ARCHITECTURE.md`. Read it before changing code and do not violate it.

## Build / test / run

```
cmake -B build && cmake --build build
ctest --test-dir build --output-on-failure   # all tests
./build/tests DivMod                         # only tests whose name contains "DivMod"
./build/rsa                                  # 1024-bit keygen + encrypt/decrypt demo
```

- Memory-error checking: configure with `-DCMAKE_CXX_FLAGS="-fsanitize=address,undefined"` (tests take ~1 s under sanitizers; the demo is fast only in a Release build).
- New behavior needs a new test. A new test file must be listed in the `tests` target in `CMakeLists.txt`.

## Code style

- C++20, compiler flags `-Wall -Wextra -Wpedantic`. The tree builds warning-free; keep it that way.
- Follow the existing naming and formatting of the surrounding code.

## Red lines

- No external libraries (tests included).
- Do not describe or present this code as secure; it is textbook RSA without padding.

## Git workflow

Keep the commit history clean. Prefer small, descriptive commits at logical points over one huge commit: one commit is one logical change, and refactors, behavior changes and tests go in separate commits. The goal is that a reviewer can digest the code step by step. The subject line describes the change; the body explains why.

- Never commit or push directly to `main`. Work on a topic branch named `<type>/<short-description>` (e.g. `docs/architecture-split`), push it, open a PR with `gh pr create` (a Summary and a Test plan), and merge through the PR. Delete the branch after merging.
- Commit messages and PR descriptions are written in English.
