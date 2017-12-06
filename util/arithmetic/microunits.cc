#include "util/arithmetic/microunits.h"

namespace micro {

int64 SqrtU(int64 value_u) {
  // Scaling factor for square roots.
  constexpr int64 kSqrtScale = 1000;
  return (int64)floor(sqrt(value_u) * kSqrtScale + 0.5);
}

int64 MultiplyU(int64 val1, int64 val2_u) {
  val1 *= val2_u;
  val1 /= kOneInU;
  return val1;
}

void MultiplyU(market::proto::Container& lhs, int64 scale_u) {
  lhs *= scale_u;
  lhs /= kOneInU;
}

void MultiplyU(market::proto::Container& lhs,
               const market::proto::Container& rhs_u) {
  lhs *= rhs_u;
  lhs /= kOneInU;
}

int64 DivideU(int64 val1, int64 val2_u) {
  val1 *= kOneInU;  
  val1 /= val2_u;
  return val1;
}

}  // namespace micro
