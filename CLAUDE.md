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

Keep the commit history clean. Prefer small, descriptive commits at logical points over one huge commit: one commit is one logical change, and refactors, behavior changes and tests go in separate commits. The goal is that a reviewer can digest the code step by step.

- Never commit or push directly to `main`. Work on a topic branch named `<type>/<short-description>` (e.g. `docs/architecture-split`), push it, open a PR with `gh pr create` (a Summary and a Test plan), and merge through the PR. Delete the branch after merging.
- Commit messages and PR descriptions are written in English.

### Commit messages

Conventional Commits 1.0, written in English:

- Subject: `<type>(<optional-scope>): <imperative summary>`. Imperative mood ("add", not "added"), lowercase after the colon, at most 72 chars, no trailing period. The scope names the module or area (e.g. `ui`, `parser`).
- Types: `feat`, `fix`, `perf`, `refactor`, `docs`, `test`, `build`, `ci`, `chore`, `style`, `revert`.
- Body (after a blank line, wrapped at ~72 cols): explains what and why, not how. Mandatory beyond trivial one-liners.
- Breaking change: `!` after the type/scope plus a `BREAKING CHANGE:` footer. Reference issues in footers (`Fixes #123`, `Refs #123`).
- Every commit is one logical change, builds, and passes the tests. Do not leave WIP or fixup commits in a PR; amend them before pushing.
- Never commit secrets, build output, or unrelated formatting churn.
- The PR title follows the same format; the PR body has a Summary (what and why) and a Test plan.

### Releases

- Versioning: SemVer 2.0 `MAJOR.MINOR.PATCH`. A `feat` bumps MINOR, a `fix`/`perf` bumps PATCH, a breaking change (`!` / `BREAKING CHANGE:`) bumps MAJOR. While below 1.0.0, breaking changes bump MINOR. Pre-releases use `-rc.1`, `-beta.1` suffixes.
- Tags: annotated and prefixed with `v` (`git tag -a v1.2.0 -m "v1.2.0"`), created only on the default branch after the release PR is merged. Never move or delete a published tag; ship a new patch instead.
- Changelog: keep `CHANGELOG.md` in Keep a Changelog format (`Added`, `Changed`, `Deprecated`, `Removed`, `Fixed`, `Security`, newest first, `Unreleased` on top), derived from the Conventional Commits since the last tag.
- Release PR: a dedicated PR titled `chore(release): vX.Y.Z` that only bumps the version (where the project defines one) and moves `Unreleased` into the new version section.
- GitHub Release: `gh release create vX.Y.Z --title "vX.Y.Z" --notes-file <notes>`, notes taken from the changelog section; mark pre-releases with `--prerelease`.
- Tagging, pushing tags and publishing releases are outward-facing: do them only when the user explicitly asks.
