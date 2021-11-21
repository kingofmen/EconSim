#include "util/arithmetic/bits.h"

#include <cstdarg>

namespace bits {

Mask MakeMask(unsigned int count, ...) {
  std::va_list args;
  va_start(args, count);
  Mask mask;
  for (unsigned int i = 0; i < count; ++i) {
    unsigned int idx = va_arg(args, unsigned int);
    if (idx > 31) {
      continue;
    }
    mask.set(idx);
  }
  va_end(args);
  return mask;
}

Mask GetMask(unsigned int m1) {
  Mask mask;
  mask.set(m1);
  return mask;
}
Mask GetMask(unsigned int m1, unsigned int m2) {
  Mask mask;
  mask.set(m1);
  mask.set(m2);
  return mask;
}
Mask GetMask(unsigned int m1, unsigned int m2, unsigned int m3) {
  Mask mask;
  mask.set(m1);
  mask.set(m2);
  mask.set(m3);
  return mask;
}
Mask GetMask(unsigned int m1, unsigned int m2, unsigned int m3,
             unsigned int m4) {
  Mask mask;
  mask.set(m1);
  mask.set(m2);
  mask.set(m3);
  mask.set(m4);
  return mask;
}
Mask GetMask(unsigned int m1, unsigned int m2, unsigned int m3, unsigned int m4,
             unsigned int m5) {
  Mask mask;
  mask.set(m1);
  mask.set(m2);
  mask.set(m3);
  mask.set(m4);
  mask.set(m5);
  return mask;
}

Mask GetMask(std::vector<unsigned int> ms) {
  Mask mask;
  for (const auto& m : ms) {
    mask.set(m);
  }
  return mask;
}


bool Subset(const Mask& cand, const Mask& super) {
  return ((cand ^ super) & cand).none();
}



} // namespace bits
