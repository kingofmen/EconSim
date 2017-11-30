#include "util/arithmetic/microunits.h"

namespace micro {

int64 SqrtU(int64 value_u) {
  // Scaling factor for square roots.
  constexpr int64 kSqrtScale = 1000;
  return (int64)floor(sqrt(value_u) * kSqrtScale + 0.5);
}

int64 MultiplyU(int64 val1_u, int64 val2_u) {
  val1_u *= val2_u;
  val1_u /= kOneInU;
  return val1_u;
}

void MultiplyU(market::proto::Container& lhs_u, int64 scale_u) {
  lhs_u *= scale_u;
  lhs_u /= kOneInU;
}

void Multiply(market::proto::Container& lhs, int64 scale_u) {
  lhs *= kOneInU;
  MultiplyU(lhs, scale_u);
  lhs /= kOneInU;
}

int64 DivideU(int64 val1_u, int64 val2_u) {
  val1_u *= kOneInU;  
  val1_u /= val2_u;
  return val1_u;
}

}  // namespace micro
