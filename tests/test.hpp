#pragma once
#include <functional>
#include <iostream>
#include <string>
#include <vector>

// Minimal in-repo test harness (no external dependencies).
struct TestCase {
  std::string name;
  std::function<void()> fn;
};

std::vector<TestCase> &Registry();
int &FailureCount();

struct Registrar {
  Registrar(const std::string &name, std::function<void()> fn) {
    Registry().push_back({name, std::move(fn)});
  }
};

#define TEST(name)                                                             \
  static void name();                                                          \
  static Registrar registrar_##name(#name, name);                              \
  static void name()

#define CHECK(cond)                                                            \
  do {                                                                         \
    if (!(cond)) {                                                             \
      std::cerr << "  FAIL " << __FILE__ << ":" << __LINE__ << ": " #cond      \
                << "\n";                                                       \
      ++FailureCount();                                                        \
    }                                                                          \
  } while (0)

#define CHECK_THROWS(expr, ExType)                                             \
  do {                                                                         \
    bool threw_ = false;                                                       \
    try {                                                                      \
      (void)(expr);                                                            \
    } catch (const ExType &) {                                                 \
      threw_ = true;                                                           \
    } catch (...) {                                                            \
    }                                                                          \
    if (!threw_) {                                                             \
      std::cerr << "  FAIL " << __FILE__ << ":" << __LINE__                    \
                << ": expected " #ExType " from " #expr "\n";                  \
      ++FailureCount();                                                        \
    }                                                                          \
  } while (0)
