#include "util/arithmetic/microunits.h"

#include "src/google/protobuf/stubs/int128.h"

namespace micro {
namespace {

constexpr uint64 kMostSigBit = 0x8000000000000000u;
constexpr uint64 kLow63Bits  = 0x7fffffffffffffffu;

uint64 Unsigned64Abs(int64 val) {
  if (val >= 0) {
    return (uint64)val;
  }
  if (val > std::numeric_limits<int64>::min()) {
    val *= -1;
    return (uint64)val;
  }
  return kMostSigBit;
}

}  // namespace


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

int64 DivideU(int64 val1, int64 val2_u, uint64* overflow) {
  if (val2_u == 0) {
    if (overflow != nullptr) {
      *overflow = (kMostSigBit | kLow63Bits);
    }
    return 0;
  }
  google::protobuf::uint128 bignum1(Unsigned64Abs(val1));
  google::protobuf::uint128 bignum2(Unsigned64Abs(val2_u));
  bignum1 *= google::protobuf::uint128((uint64)kOneInU);
  bignum1 /= bignum2;

  int sign = 1;
  if (val1 < 0) sign *= -1;
  if (val2_u < 0) sign *= -1;
  uint64 lo_bits = google::protobuf::Uint128Low64(bignum1);
  if (overflow != nullptr) {
    *overflow = google::protobuf::Uint128High64(bignum1);
    if (lo_bits & kMostSigBit) {
      // Indicates overflow except in one special case, when the value is
      // precisely the lowest representable signed 64-bit integer.
      if (lo_bits & kLow63Bits || sign > 0) {
        *overflow = (*overflow | kMostSigBit);
      }
    }
  }

  int64 ret = lo_bits;
  ret *= sign;
  return ret;
}

}  // namespace micro
