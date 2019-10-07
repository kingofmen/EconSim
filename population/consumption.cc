#include "population/consumption.h"

#include "absl/strings/substitute.h"
#include "market/proto/goods.pb.h"
#include "market/goods_utils.h"
#include "population/proto/consumption.pb.h"
#include "util/arithmetic/microunits.h"
#include "util/headers/int_types.h"
#include "util/status/status.h"

namespace consumption {

const int kMaxSubstitutables = 3;

util::Status internal_optimum_1(const market::proto::Container& goods,
                                const market::proto::Container& prices,
                                int64 offset_u, int64 dsquared_u,
                                market::proto::Container* result) {
  if (goods.quantities_size() != 1) {
    return util::InvalidArgumentError(
        absl::Substitute("Optimum expected 1 good, got $0: $1",
                         goods.quantities_size(), goods.DebugString()));
  }
  result->Clear();
  *result += goods;

  return util::OkStatus();
}
util::Status internal_optimum_2(const market::proto::Container& goods,
                                const market::proto::Container& prices,
                                int64 offset_u, int64 dsquared_u,
                                market::proto::Container* result) {
  return util::OkStatus();
}
util::Status internal_optimum_3(const market::proto::Container& goods,
                                const market::proto::Container& prices,
                                int64 offset_u, int64 dsquared_u,
                                market::proto::Container* result) {
  return util::OkStatus();
}

namespace {
util::Status internal_optimum(const market::proto::Container& goods,
                              const market::proto::Container& prices,
                              int64 offset_u, int64 dsquared_u,
                              market::proto::Container* result) {
  switch (goods.quantities_size()) {
    case 1:
      return internal_optimum_1(goods, prices, offset_u, dsquared_u, result);
    case 2:
      return internal_optimum_2(goods, prices, offset_u, dsquared_u, result);
    case 3:
      return internal_optimum_3(goods, prices, offset_u, dsquared_u, result);
    default:
      return util::InvalidArgumentError(
          absl::Substitute("Optimum for $0 goods, can handle at most $1",
                           goods.quantities_size(), kMaxSubstitutables));
  }
  return util::OkStatus();
}

} // namespace

util::Status Optimum(const proto::Substitutes& subs,
                     const market::proto::Container& prices,
                     market::proto::Container* result) {
  return internal_optimum(subs.consumed(), prices, subs.offset_u(),
                          subs.min_amount_square_u(), result);
}

util::Status Validate(const proto::Substitutes& subs) {
  int numCon  = subs.consumed().quantities_size();
  int numCap = subs.movable_capital().quantities_size();
  if (0 == numCon + numCap) {
    return util::InvalidArgumentError(
        absl::Substitute("$0: Must specify at least one good", subs.name()));
  }
  if (kMaxSubstitutables < numCon) {
    return util::InvalidArgumentError(
        absl::Substitute("$0: Can handle at most $1 consumables, found $2",
                         subs.name(), kMaxSubstitutables, numCon));
  }
  if (kMaxSubstitutables < numCap) {
    return util::InvalidArgumentError(
        absl::Substitute("$0: Can handle at most $1 capital, found $2",
                         subs.name(), kMaxSubstitutables, numCap));
  }

  int oPowN = micro::kOneInU;
  for (int i = 0; i < numCon; ++i) {
    oPowN = micro::MultiplyU(oPowN, subs.offset_u());
  }
  if (subs.min_amount_square_u() <= oPowN) {
    return util::InvalidArgumentError(absl::Substitute(
        "$0 consumables: D^2 must be strictly greater than o^n, found $1 <= $2",
        subs.name(), subs.min_amount_square_u(), oPowN));
  }
  oPowN = 1;
  for (int i = 0; i < numCap; ++i) {
    oPowN = micro::MultiplyU(oPowN, subs.offset_u());
  }
  if (subs.min_amount_square_u() <= oPowN) {
    return util::InvalidArgumentError(absl::Substitute(
        "$0 capital: D^2 must be strictly greater than o^n, found $1 <= $2",
        subs.name(), subs.min_amount_square_u(), oPowN));
  }

  return util::OkStatus();
}

}  // namespace consumption
