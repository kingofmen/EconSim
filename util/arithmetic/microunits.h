// Methods for manipulating quantities expressed in micro-units.
#ifndef UTIL_ARITHMETIC_MICROUNITS_H
#define UTIL_ARITHMETIC_MICROUNITS_H

#include <limits>
#include <string>

#include "games/market/goods_utils.h"
#include "games/market/proto/goods.pb.h"
#include "util/headers/int_types.h"

namespace micro {

// Powers of ten in micro-units.
constexpr int64 kOneInU      = 1000000;
constexpr int64 kTenInU      = 10 * kOneInU;
constexpr int64 kHundredInU  = 100 * kOneInU;
constexpr int64 kThousandInU = 1000 * kOneInU;
constexpr int64 kMaxU        = std::numeric_limits<int64>::max();

// Some fractions.
constexpr int64 kOneTenthInU     = kOneInU * 1 / 10;
constexpr int64 kOneHundredthInU = kOneInU * 1 / 100;
constexpr int64 kOneThirdInU     = 333333;
constexpr int64 kTwoThirdsInU    = 666667;
constexpr int64 kOneFourthInU    = kOneInU * 1 / 4;
constexpr int64 kHalfInU         = kOneInU * 1 / 2;
constexpr int64 kThreeFourthsInU = kOneInU * 3 / 4;

// Returns the square root of value_u in micro-units.
int64 SqrtU(int64 value_u);

// Returns the square of value_u in micro-units.
int64 SquareU(int64 value_u);

// Returns the cube of value_u in micro-units.
int64 CubeU(int64 value_u);

// Returns the nth root of value_u.
int64 NRootU(int n, int64 value_u);

// Returns b_u raised to the nth power.
int64 PowU(int64 b_u, int n);

// The MultiplyU methods return products that maintain the scale of the
// left-hand value.

// Integer multiplication.
int64 MultiplyU(int64 val1, int64 val2_u);
int64 MultiplyU(int64 val1, int64 val2_u, int64 val3_u);

// In-place scaling a container.
void MultiplyU(market::proto::Container& lhs, int64 scale_u);

// In-place matrix-vector multiplication - not dot product.
void MultiplyU(market::proto::Container& lhs,
               const market::proto::Container& rhs_u);

// DivideU methods return ratios, maintaining the left-hand scale.

// Integer division. If the result overflows and overflow is non-null, overflow
// will be set to a nonzero value. If the result is considered as a 128-bit
// unsigned int, the MSB of overflow stores its 64th bit, and the least
// significant bits store the 65th through 127th. Note that if val2_u is
// expressed in micro-units, as it should be, then the maximum possible overflow
// is gotten by dividing the lowest representable int64 value by 1 micro-unit,
// which requires considerably less than 63 bits to represent.
// If val2_u is zero, zero will be returned; if overflow is non-null all its
// bits will be set.
int64 DivideU(int64 val1, int64 val2_u, uint64* overflow = nullptr);

// Returns a human-readable string, that is, in units rather than micro-units.
// Note that the rounding is truncation.
std::string DisplayString(market::Measure amount, int digits);
std::string DisplayString(const market::proto::Quantity& q, int digits);


}  // namespace micro

#endif
