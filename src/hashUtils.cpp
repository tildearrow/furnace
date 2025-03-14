#include "hashUtils.h"
#include <cmath>
#include <cstddef>
#include <functional>

constexpr size_t computePi() {
  // 64-bit fixed point integer of pi, used as the constant in the hash combining function
  // due to it being a irrational number containing close-to-equal distribution of bits
  // also it's pi day
  return 0x517cc1b727220a94 >> (64 - (sizeof(size_t) * 8));
}

template <typename T>
size_t combineHash(size_t curValue, const T& value) {
  std::hash<T> hash;
  curValue ^= hash(value) + computePi() + (curValue << 6) + (curValue >> 2);
  return curValue;
}
