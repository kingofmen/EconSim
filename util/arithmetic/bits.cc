#include "util/arithmetic/bits.h"

#include <cstdarg>

namespace bits {

Mask MakeMask(unsigned int count, ...) {
  std::va_list args;
  va_start(args, count);
  Mask mask;
  for (unsigned int i = 0; i < count; ++i) {
    unsigned int idx = va_arg(args, unsigned int);
    if (idx > 32 || idx == 0) {
      continue;
    }
    mask.set(idx-1);
  }
  va_end(args);
  return mask;
}

} // namespace bits
