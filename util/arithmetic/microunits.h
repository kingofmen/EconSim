// Methods for manipulating quantities expressed in micro-units.
#ifndef UTIL_ARITHMETIC_MICROUNITS_H
#define UTIL_ARITHMETIC_MICROUNITS_H

#include "util/headers/int_types.h"
#include "market/goods_utils.h"
#include "market/proto/goods.pb.h"

namespace micro {

// A unit in micro-units.
constexpr int64 kOneInU = 1000000;

// Returns the square root of value_u in micro-units.
int64 SqrtU(int64 value_u);

// Returns the product, in micro-units, of two values specified in micro-units.
int64 MultiplyU(int64 val1_u, int64 val2_u);

// Multiplies a container specified in micro-units by a scale in micro-units.
void MultiplyU(market::proto::Container& lhs_u, int64 scale_u);

// Multiplies a container by a scale in micro-units.
void Multiply(market::proto::Container& lhs, int64 scale_u);

// Returns the ratio of two quantities in micro-units.
int64 DivideU(int64 val1_u, int64 val2_u);

}  // namespace micro

#endif
