#include "population/consumption.h"

#include "absl/strings/substitute.h"
#include "market/proto/goods.pb.h"
#include "market/goods_utils.h"
#include "population/proto/consumption.pb.h"
#include "util/arithmetic/microunits.h"
#include "util/headers/int_types.h"
#include "util/logging/logging.h"
#include "util/status/status.h"

namespace consumption {

const int kMaxSubstitutables = 3;

util::Status calcX(int64 a, int64 b, int64 px, int64 py, int64 dsquared_u,
            int64 offset_u, int64* value) {
  uint64 overflow = 0;
  int64 aC = micro::MultiplyU(py, dsquared_u);
  aC = micro::MultiplyU(aC, a);
  aC = micro::DivideU(aC, micro::MultiplyU(b, px), &overflow);
  if (overflow != 0) {
    return util::InvalidArgumentError(absl::Substitute(
        "Division overflow with a=$0, b=$1, px=$2, py = $3, aC = $4", a, b, px,
        py, aC));
  }
  VLOGF(5, "Coefficients %d, %d => aC = %d", a, b, aC);

  int64 radical = micro::SqrtU(aC);
  VLOGF(5, "Square root %d", radical);
  *value = micro::DivideU(radical - offset_u, a);
  if (overflow != 0) {
    return util::InvalidArgumentError(absl::Substitute(
        "Division overflow with radical=$0, offset = $1, a = $2", radical,
        offset_u, a));
  }
  return util::OkStatus();
}

int64 calcCoefficient(int64 dsquared_u, int64 offset_u, int numGoods,
                      int64 crossing_u) {
  auto nom = dsquared_u - micro::PowU(offset_u, numGoods);
  auto denom = micro::MultiplyU(crossing_u, micro::PowU(offset_u, numGoods - 1));
  return micro::DivideU(nom, denom);
}

util::Status internal_optimum_1(const market::proto::Quantity& good,
                                const market::proto::Container& prices,
                                int64 offset_u, int64 dsquared_u,
                                market::proto::Container* result) {
  result->Clear();
  *result += good;
  return util::OkStatus();
}

util::Status
internal_optimum_2(const std::vector<market::proto::Quantity>& goods,
                   const market::proto::Container& prices, int64 offset_u,
                   int64 dsquared_u, market::proto::Container* result) {
  if (goods.size() != 2) {
    return util::InvalidArgumentError(
        absl::Substitute("Optimum expected 2 goods, got $0", goods.size()));
  }

  result->Clear();
  market::proto::Container coeffs;
  // TODO: Coefficients are constant, so can be calculated once and cached.
  int64 a = calcCoefficient(dsquared_u, offset_u, 2, goods[0].amount());
  int64 b = calcCoefficient(dsquared_u, offset_u, 2, goods[1].amount());
  int64 px = market::GetAmount(prices, goods[0].kind());
  int64 py = market::GetAmount(prices, goods[1].kind());
  VLOGF(5, "Calculating %s/%s with prices %d, %d", goods[0].kind(), goods[1].kind(), px, py);

  int64 xValue = 0;
  int64 yValue = 0;
  auto status = calcX(a, b, px, py, dsquared_u, offset_u, &xValue);
  if (!status.ok()) {
    return status;
  }
  status = calcX(b, a, py, px, dsquared_u, offset_u, &yValue);
  if (!status.ok()) {
    return status;
  }

  if (xValue < 0 && yValue < 0) {
    // This should never happen.
    return util::InvalidArgumentError(
        absl::Substitute("Two negative amounts for $0 and $1", goods[0].kind(),
                         goods[1].kind()));
  }
  if (xValue < 0) {
    return internal_optimum_1(goods[1], prices, offset_u, dsquared_u, result);
  }
  if (yValue < 0) {
    return internal_optimum_1(goods[0], prices, offset_u, dsquared_u, result);
  }

  market::SetAmount(goods[0].kind(), xValue, result);
  market::SetAmount(goods[1].kind(), yValue, result);

  return util::OkStatus();
}

util::Status
internal_optimum_3(const std::vector<market::proto::Quantity>& goods,
                   const market::proto::Container& prices, int64 offset_u,
                   int64 dsquared_u, market::proto::Container* result) {
  if (goods.size() != 3) {
    return util::InvalidArgumentError(
        absl::Substitute("Optimum expected 3 goods, got $0", goods.size()));
  }

  result->Clear();
  int64 coefficients[3];
  for (int i = 0; i < 3; ++i) {
    coefficients[i] =
        calcCoefficient(dsquared_u, offset_u, 3, goods[i].amount());
  }

  uint64 overflow;
  for (int i = 0; i < 3; ++i) {
    int j = (i + 1) % 3;
    int k = (i + 2) % 3;
    int64 priceI = market::GetAmount(prices, goods[i].kind());
    int64 priceJ = market::GetAmount(prices, goods[j].kind());
    int64 priceK = market::GetAmount(prices, goods[k].kind());
    int64 priceFrac = micro::MultiplyU(priceJ, priceK);
    priceFrac = micro::DivideU(priceFrac, micro::SquareU(priceI), &overflow);
    if (overflow != 0) {
      return util::InvalidArgumentError(
          absl::Substitute("Overflow in price-ratio calculation $0: $1, $2, $3",
                           i, priceI, priceJ, priceK));
    }
    int64 coefFrac =
        micro::MultiplyU(dsquared_u, micro::SquareU(coefficients[i]));
    coefFrac = micro::DivideU(
        coefFrac, micro::MultiplyU(coefficients[j], coefficients[k]),
        &overflow);
    if (overflow != 0) {
      return util::InvalidArgumentError(
          absl::Substitute("Overflow in coefficient-ratio calculation $0: $1, $2, $3",
                           i, coefficients[i], coefficients[j], coefficients[k]));
    }
    int64 root = micro::NRootU(3, micro::MultiplyU(coefFrac, priceFrac));
    root -= offset_u;
    int64 xValue = micro::DivideU(root, coefficients[i], &overflow);
    if (overflow != 0) {
      return util::InvalidArgumentError(absl::Substitute(
          "Overflow in final division $0: $1 / $2", i, root, coefficients[i]));
    }

    if (xValue < 0) {
      std::vector<market::proto::Quantity> copy;
      copy.push_back(goods[j]);
      copy.push_back(goods[k]);
      return internal_optimum_2(copy, prices, offset_u, dsquared_u, result);
    }

    market::SetAmount(goods[i].kind(), xValue, result);
  }  

  return util::OkStatus();
}

namespace {
util::Status internal_optimum(const market::proto::Container& goods,
                              const market::proto::Container& prices,
                              int64 offset_u, int64 dsquared_u,
                              market::proto::Container* result) {
  auto expanded = market::Expand(goods);
  switch (expanded.size()) {
    case 1:
      return internal_optimum_1(expanded[0], prices, offset_u, dsquared_u,
                                result);
    case 2:
      return internal_optimum_2(expanded, prices, offset_u, dsquared_u, result);
    case 3:
      return internal_optimum_3(expanded, prices, offset_u, dsquared_u, result);
    default:
      return util::InvalidArgumentError(
          absl::Substitute("Optimum for $0 goods, can handle at most $1",
                           goods.quantities().size(), kMaxSubstitutables));
  }
  return util::OkStatus();
}

} // namespace

util::Status Optimum(const proto::Substitutes& subs,
                     const market::proto::Container& prices,
                     market::proto::Container* result) {
  for (const auto& price : prices.quantities()) {
    if (price.second < 1) {
      return util::InvalidArgumentError(
          absl::Substitute("$0: Prices must be positive, found $1 for $2",
                           subs.name(), price.second, price.first));
    }
  }
  return internal_optimum(subs.consumed(), prices, subs.offset_u(),
                          subs.min_amount_square_u(), result);
}

util::Status Consumption(const proto::Substitutes& subs,
                         const market::proto::Container& prices,
                         const market::AvailabilityEstimator& available,
                         market::proto::Container* result) {
  for (const auto& price : prices.quantities()) {
    if (price.second < 1) {
      return util::InvalidArgumentError(
          absl::Substitute("$0: Prices must be positive, found $1 for $2",
                           subs.name(), price.second, price.first));
    }
  }


  auto status = internal_optimum(subs.consumed(), prices, subs.offset_u(),
                                 subs.min_amount_square_u(), result);

  return status;
}


util::Status Validate(const proto::Substitutes& subs) {
  if (subs.offset_u() <= 0) {
    return util::InvalidArgumentError(
        absl::Substitute("$0: Offset must be positive, found $1",
                         subs.name(), subs.offset_u()));
  }

  int numCon  = subs.consumed().quantities().size();
  int numCap = subs.movable_capital().quantities().size();
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

  int64 dsquared_u = subs.min_amount_square_u();
  int64 offset_u = subs.offset_u();
  int oPowN = micro::kOneInU;
  for (int i = 0; i < numCon; ++i) {
    oPowN = micro::MultiplyU(oPowN, offset_u);
  }
  if (dsquared_u <= oPowN) {
    return util::InvalidArgumentError(absl::Substitute(
        "$0 consumables: D^2 must be strictly greater than o^n, found $1 <= $2",
        subs.name(), dsquared_u, oPowN));
  }
  oPowN = 1;
  for (int i = 0; i < numCap; ++i) {
    oPowN = micro::MultiplyU(oPowN, offset_u);
  }
  if (dsquared_u <= oPowN) {
    return util::InvalidArgumentError(absl::Substitute(
        "$0 capital: D^2 must be strictly greater than o^n, found $1 <= $2",
        subs.name(), dsquared_u, oPowN));
  }

  for (const auto& good : subs.consumed().quantities()) {
    auto nom = dsquared_u - micro::MultiplyU(offset_u, offset_u);
    auto denom = micro::MultiplyU(good.second, offset_u);
    uint64 overflow = 0;
    auto ratio = micro::DivideU(nom, denom, &overflow);
    if (overflow != 0) {
      return util::InvalidArgumentError(absl::Substitute(
          "$0 $1 (consumed): Coefficient ratio overflows, increase the "
          "offset or crossing point",
          subs.name(), good.first));
    }
    if (ratio < 1) {
      return util::InvalidArgumentError(absl::Substitute(
          "$0 $1 (consumed): Nonpositive coefficient ratio, D^2=$2, o=$3, "
          "x_0=$4, "
          "increase minimum or decrease offset",
          subs.name(), good.first, dsquared_u, offset_u, good.second));
    }
  }

  for (const auto& good : subs.movable_capital().quantities()) {
    auto nom = dsquared_u - micro::MultiplyU(offset_u, offset_u);
    auto denom = micro::MultiplyU(good.second, offset_u);
    uint64 overflow = 0;
    auto ratio = micro::DivideU(nom, denom, &overflow);
    if (overflow != 0) {
      return util::InvalidArgumentError(absl::Substitute(
          "$0 $1 (capital): Coefficient ratio overflows, increase the "
          "offset or crossing point",
          subs.name(), good.first));
    }
    if (ratio < 1) {
      return util::InvalidArgumentError(
          absl::Substitute("$0 $1 (capital): Nonpositive coefficient ratio, "
                           "increase minimum or decrease offset",
                           subs.name(), good.first));
    }
  }
  
  return util::OkStatus();
}

}  // namespace consumption
