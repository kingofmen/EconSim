#include "games/population/consumption.h"

#include "absl/strings/substitute.h"
#include "games/market/goods_utils.h"
#include "games/market/proto/goods.pb.h"
#include "games/population/proto/consumption.pb.h"
#include "util/arithmetic/microunits.h"
#include "util/headers/int_types.h"
#include "util/logging/logging.h"
#include "util/status/status.h"

namespace consumption {

const int kMaxSubstitutables = 3;
uint64 overflow;

util::Status calcX(int64 a, int64 b, int64 px, int64 py, int64 dsquared_u,
                   int64 offset_u, int64* value) {
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
  *value = micro::DivideU(radical - offset_u, a, &overflow);
  if (overflow != 0) {
    return util::InvalidArgumentError(absl::Substitute(
        "Division overflow with radical=$0, offset = $1, a = $2", radical,
        offset_u, a));
  }
  return util::OkStatus();
}

util::Status internal_optimum_1(const market::proto::Quantity& good,
                                const market::proto::Container& coefs,
                                const market::proto::Container& prices,
                                int64 offset_u, int64 dsquared_u,
                                market::proto::Container* result) {
  result->Clear();
  auto coef = market::GetAmount(coefs, good.kind());
  VLOGF(3, "D^2 %d, offset %d, coef %d", dsquared_u, offset_u, coef);
  micro::Measure amount = dsquared_u - offset_u;
  amount = micro::DivideU(amount, coef);
  market::Add(good.kind(), amount, result);
  return util::OkStatus();
}

util::Status
internal_optimum_2(const std::vector<market::proto::Quantity>& goods,
                   const market::proto::Container& coefs,
                   const market::proto::Container& prices, int64 offset_u,
                   int64 dsquared_u, market::proto::Container* result) {
  if (goods.size() != 2) {
    return util::InvalidArgumentError(
        absl::Substitute("Optimum expected 2 goods, got $0", goods.size()));
  }

  result->Clear();
  market::proto::Container coeffs;
  int64 a = market::GetAmount(coefs, goods[0].kind());
  int64 b = market::GetAmount(coefs, goods[1].kind());
  int64 px = market::GetAmount(prices, goods[0].kind());
  int64 py = market::GetAmount(prices, goods[1].kind());
  VLOGF(5, "Calculating %s/%s with prices %d, %d", goods[0].kind(),
        goods[1].kind(), px, py);

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
    return internal_optimum_1(goods[1], coefs, prices, offset_u, dsquared_u,
                              result);
  }
  if (yValue < 0) {
    return internal_optimum_1(goods[0], coefs, prices, offset_u, dsquared_u,
                              result);
  }

  market::SetAmount(goods[0].kind(), xValue, result);
  market::SetAmount(goods[1].kind(), yValue, result);

  return util::OkStatus();
}

util::Status
internal_optimum_3(const std::vector<market::proto::Quantity>& goods,
                   const market::proto::Container& coefs,
                   const market::proto::Container& prices, int64 offset_u,
                   int64 dsquared_u, market::proto::Container* result) {
  if (goods.size() != 3) {
    return util::InvalidArgumentError(
        absl::Substitute("Optimum expected 3 goods, got $0", goods.size()));
  }

  result->Clear();
  int64 coefficients[3];
  for (int i = 0; i < 3; ++i) {
    coefficients[i] = market::GetAmount(coefs, goods[i].kind());
  }

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
      return util::InvalidArgumentError(absl::Substitute(
          "Overflow in coefficient-ratio calculation $0: $1, $2, $3", i,
          coefficients[i], coefficients[j], coefficients[k]));
    }
    int64 root = micro::NRootU(3, micro::MultiplyU(coefFrac, priceFrac));
    root -= offset_u;
    int64 xValue = micro::DivideU(root, coefficients[i], &overflow);
    if (overflow != 0) {
      return util::InvalidArgumentError(absl::Substitute(
          "Overflow in final division $0: $1 / $2", i, root, coefficients[i]));
    }

    if (xValue < 0) {
      // Clamp this good to zero, recalculating D^2 for the reduced call.
      auto reduced_dsquared_u = micro::DivideU(dsquared_u, offset_u);
      std::vector<market::proto::Quantity> copy;
      copy.push_back(goods[j]);
      copy.push_back(goods[k]);
      return internal_optimum_2(copy, coefs, prices, offset_u,
                                reduced_dsquared_u, result);
    }

    market::SetAmount(goods[i].kind(), xValue, result);
  }

  return util::OkStatus();
}

namespace {
util::Status internal_optimum(const market::proto::Container& goods,
                              const market::proto::Container& coefs,
                              const market::proto::Container& prices,
                              int64 offset_u, int64 dsquared_u,
                              market::proto::Container* result) {
  auto expanded = market::Expand(goods);
  switch (expanded.size()) {
  case 1:
    return internal_optimum_1(expanded[0], coefs, prices, offset_u, dsquared_u,
                              result);
  case 2:
    return internal_optimum_2(expanded, coefs, prices, offset_u, dsquared_u,
                              result);
  case 3:
    return internal_optimum_3(expanded, coefs, prices, offset_u, dsquared_u,
                              result);
  default:
    return util::InvalidArgumentError(
        absl::Substitute("Optimum for $0 goods, can handle at most $1",
                         goods.quantities().size(), kMaxSubstitutables));
  }
  return util::OkStatus();
}

// Clamp one good at a time to its constraint and recalculate. One at a time
// because clamping may cause the new solution to run into other constraints.
util::Status constrained_optimum(const market::proto::Container& goods,
                                 const market::proto::Container& coefs,
                                 const market::proto::Container& maxima,
                                 const market::proto::Container& minima,
                                 const market::AvailabilityEstimator& available,
                                 const market::proto::Container& prices,
                                 int64 offset_u, int64 dsquared_u,
                                 market::proto::Container* result) {
  int numGoods = goods.quantities().size();
  if (numGoods == 0) {
    return util::NotFoundError("No degrees of freedom left");
  }

  market::proto::Quantity constraint;
  for (const auto& good : goods.quantities()) {
    if (!market::Contains(maxima, good.first)) {
      continue;
    }
    micro::Measure con = market::GetAmount(maxima, good);
    if (market::GetAmount(*result, good) - con > constraint.amount()) {
      constraint = market::MakeQuantity(good.first, con);
    }
  }

  if (constraint.amount() == 0) {
    for (const auto& good : goods.quantities()) {
      if (!market::Contains(minima, good.first)) {
        continue;
      }
      micro::Measure con = market::GetAmount(minima, good);
      if (con - market::GetAmount(*result, good) > constraint.amount()) {
        constraint = market::MakeQuantity(good.first, con);
      }
    }
  }
  if (constraint.kind().empty()) {
    return util::InvalidArgumentError(
        absl::Substitute("Constrained optimum could not find a constraint"));
  }

  market::proto::Container free;
  market::proto::Container free_result;
  free += goods;
  market::Erase(constraint, &free);

  micro::Measure reduced_dsquared_u = dsquared_u;
  auto coef = market::GetAmount(coefs, constraint.kind());
  coef = micro::MultiplyU(coef, constraint.amount());
  coef += offset_u;
  reduced_dsquared_u = micro::DivideU(reduced_dsquared_u, coef);
  VLOG(3,
       absl::Substitute("$0 constrained to $1, reduced D^2 to $2",
                        constraint.kind(), constraint.amount(),
                        reduced_dsquared_u));
  auto status = internal_optimum(free, coefs, prices, offset_u,
                                 reduced_dsquared_u, &free_result);
  if (!status.ok()) {
    return status;
  }

  free_result += constraint;
  if (available.AvailableImmediately(free_result) && minima <= free_result) {
    result->Clear();
    *result << free_result;
    return util::OkStatus();
  }

  market::proto::Container new_minima;
  market::Copy(minima, free_result, &new_minima);
  status = constrained_optimum(free, coefs, maxima, new_minima, available,
                               prices, offset_u, reduced_dsquared_u, result);
  if (!status.ok()) {
    return status;
  }

  *result += constraint;
  return util::OkStatus();
}

// Greedy-local algorithm simply takes everything available of each good in
// succession until constraints are satisfied or no more goods remain.
util::Status greedyLocal(const std::vector<market::proto::Quantity>& toConsume,
                         const market::proto::Container& coefs,
                         const market::AvailabilityEstimator& available,
                         int64 dsquared_u, int64 offset_u,
                         market::proto::Container* result) {
  result->Clear();
  micro::Measure product_u = micro::kOneInU;
  for (int i = 0; i < toConsume.size(); ++i) {
    const auto& tc = toConsume[i];
    auto av = available.AvailableImmediately(tc.kind());
    // Roundoff error in coefficients may cause us to miss that we definitely
    // have enough, so store this fact if it's true.
    bool sufficient = false;
    if (av >= tc.amount()) {
      av = tc.amount();
      sufficient = true;
    }

    auto remainingOffsets = micro::PowU(offset_u, toConsume.size() - i - 1);
    auto coef = market::GetAmount(coefs, tc.kind());
    micro::Measure current_u = micro::MultiplyU(coef, av) + offset_u;
    if (micro::MultiplyU(current_u, product_u, remainingOffsets) > dsquared_u) {
      current_u = micro::DivideU(dsquared_u,
                                 micro::MultiplyU(product_u, remainingOffsets));
      current_u -= offset_u;
      current_u = micro::DivideU(current_u, coef);
      av = current_u;
    }

    market::SetAmount(tc.kind(), av, result);
    if (sufficient) {
      return util::OkStatus();
    }

    current_u = micro::MultiplyU(coef, av) + offset_u;
    product_u = micro::MultiplyU(product_u, current_u);
    // Allow for some roundoff error in coefficient or amount.
    static const int64 tolerance = 10;
    if (micro::MultiplyU(product_u, remainingOffsets) + tolerance >=
        dsquared_u) {
      return util::OkStatus();
    }
  }

  // If we get here the constraints are not satisfiable with the available
  // goods.
  result->Clear();
  return util::NotFoundError(
      "Not enough goods available to satisfy greedy-local.");
}

// Calculates the coefficients for the provided substitutes.
util::Status coefficients(const proto::Substitutes& subs,
                          market::proto::Container* coefs) {
  int64 dsquared_u = subs.min_amount_square_u();
  int64 offset_u = subs.offset_u();
  int numGoods = subs.consumed().quantities().size();
  int64 nom = dsquared_u - micro::PowU(offset_u, numGoods);

  for (const auto& good : subs.consumed().quantities()) {
    int64 crossing_u = good.second;
    int64 denom =
        micro::MultiplyU(crossing_u, micro::PowU(offset_u, numGoods - 1));
    int64 coef_u = micro::DivideU(nom, denom, &overflow);
    if (overflow != 0) {
      return util::InvalidArgumentError(absl::Substitute(
          "Division overflow in coefficient of $0 with nom = $1, denom = $2",
          good.first, nom, denom));
    }
    market::SetAmount(good.first, coef_u, coefs);
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
  market::proto::Container coefs;
  coefficients(subs, &coefs);
  return internal_optimum(subs.consumed(), coefs, prices, subs.offset_u(),
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

  // Sanity-check that there is something available, before we begin expensive
  // calculations.
  auto toConsume = market::Expand(subs.consumed());
  int avCount = 0;
  for (const auto& tc : toConsume) {
    auto av = available.AvailableImmediately(tc.kind());
    auto min = market::GetAmount(subs.minimum(), tc.kind());
    if (av < min) {
      return util::NotFoundError(absl::Substitute(
          "$0 : Minimum $1 > available $2", tc.kind(), min, av));
    }
    if (av > 1) {
      avCount++;
    }
  }
  if (avCount == 0) {
    return util::NotFoundError(
        absl::Substitute("Literally no goods of $0 requested kinds available",
                         toConsume.size()));
  }

  market::proto::Container coefs;
  coefficients(subs, &coefs);

  // If only one good is available, no need for fancy optimisation.
  if (avCount == 1) {
    return greedyLocal(toConsume, coefs, available, subs.min_amount_square_u(),
                       subs.offset_u(), result);
  }

  auto status =
      internal_optimum(subs.consumed(), coefs, prices, subs.offset_u(),
                       subs.min_amount_square_u(), result);
  if (status.ok()) {
    if (available.AvailableImmediately(*result) && subs.minimum() < *result) {
      return util::OkStatus();
    }

    auto goods = market::Expand(*result);
    market::proto::Container maxima;
    for (const auto& good : goods) {
      auto canGet = available.AvailableImmediately(good.kind());
      if (canGet < good.amount()) {
        market::SetAmount(good.kind(), canGet, &maxima);
      }
    }

    status = constrained_optimum(subs.consumed(), coefs, maxima, subs.minimum(),
                                 available, prices, subs.offset_u(),
                                 subs.min_amount_square_u(), result);
    if (status.ok()) {
      return status;
    }
  }

  // Final fallback.
  return greedyLocal(toConsume, coefs, available, subs.min_amount_square_u(),
                     subs.offset_u(), result);
}

util::Status Validate(const proto::Substitutes& subs) {
  if (subs.offset_u() <= 0) {
    return util::InvalidArgumentError(absl::Substitute(
        "$0: Offset must be positive, found $1", subs.name(), subs.offset_u()));
  }

  int numCon = subs.consumed().quantities().size();
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

} // namespace consumption
