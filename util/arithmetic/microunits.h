// Methods for manipulating quantities expressed in micro-units.
#ifndef UTIL_ARITHMETIC_MICROUNITS_H
#define UTIL_ARITHMETIC_MICROUNITS_H

#include "util/headers/int_types.h"
#include "market/goods_utils.h"
#include "market/proto/goods.pb.h"

namespace micro {

// Powers of ten in micro-units.
constexpr int64 kOneInU      = 1000000;
constexpr int64 kTenInU      = 10 * kOneInU;
constexpr int64 kHundredInU  = 100 * kOneInU;
constexpr int64 kThousandInU = 1000 * kOneInU;


// Returns the square root of value_u in micro-units.
int64 SqrtU(int64 value_u);

// The MultiplyU methods return products that maintain the scale of the
// left-hand value.

// Integer multiplication.
int64 MultiplyU(int64 val1, int64 val2_u);

// In-place scaling a container.
void MultiplyU(market::proto::Container& lhs, int64 scale_u);

// In-place matrix-vector multiplication - not dot product.
void MultiplyU(market::proto::Container& lhs,
               const market::proto::Container& rhs_u);

// DivideU methods return ratios, maintaining the left-hand scale.

// Integer division.
int64 DivideU(int64 val1, int64 val2_u);

}  // namespace micro

#endif
