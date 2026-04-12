#include "BigInt.hpp"
#include <cstdint>
#include <cstdlib>

BigInt::BigInt(int64_t val) {
  isNegative = (val < 0);
  uint64_t absVal = (val == INT64_MIN) ? static_cast<uint64_t>(INT64_MAX) + 1
                                       : static_cast<uint64_t>(std::abs(val));

  if (absVal == 0) {
    limbs.push_back(0);
    return;
  }

  while (absVal > 0) {
    limbs.push_back(static_cast<uint32_t>(
        absVal & 0xFFFFFFFF)); // masking using 0xFFFFFFFF so that top 32 bits
                               // are all zeroes and the bottom 32 bits are
                               // exactly as they shoudl be

    absVal >>= 32; // after masking shift the val 32 bits to the right, carrying
                   // the rest of the 32 bits to be pushed to the vector
  }
}

BigInt::BigInt(const std::string &hexStr) {
  if (hexStr.empty()) {
    limbs.push_back(0);
    return;
  }

  size_t startIdx = 0;

  if (hexStr.starts_with('-')) {
    isNegative = true;
    startIdx = 1;
  }

  // parsing right to left to maintain little-endian order
  for (int i = hexStr.length(); i > static_cast<int>(startIdx); i -= 8) {
    int chunkStart = std::max(static_cast<int>(startIdx), i - 8);
    int chunkLength = i - chunkStart;

    std::string chunk = hexStr.substr(chunkStart, chunkLength);

    // converts hex str chunk into 32 bit integer, std::stoul converts to
    // unsigned long base 16. casted to uint32_t to be explicit.
    uint32_t limbVal = static_cast<uint32_t>(std::stoul(chunk, nullptr, 16));
    limbs.push_back(limbVal);
  }
  trim();
}

void BigInt::trim() {
  while (limbs.size() > 1 && limbs.back() == 0) {
    limbs.pop_back();
  }

  if (limbs.size() == 1 && limbs[0] == 0) {
    isNegative = false;
  }
}
