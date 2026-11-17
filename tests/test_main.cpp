#include "test.hpp"

std::vector<TestCase> &Registry() {
  static std::vector<TestCase> r;
  return r;
}
int &FailureCount() {
  static int n = 0;
  return n;
}

// Usage: tests [name-substring]  (runs only matching tests)
int main(int argc, char **argv) {
  std::string filter = argc > 1 ? argv[1] : "";
  int ran = 0;
  for (auto &t : Registry()) {
    if (t.name.find(filter) == std::string::npos)
      continue;
    std::cout << "[ RUN ] " << t.name << "\n";
    t.fn();
    ++ran;
  }
  std::cout << ran << " test(s), " << FailureCount() << " failure(s)\n";
  return FailureCount() == 0 ? 0 : 1;
}
