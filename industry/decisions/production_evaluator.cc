#include "industry/decisions/production_evaluator.h"

#include <string>

#include "absl/strings/substitute.h"
#include "market/goods_utils.h"
#include "market/market.h"
#include "util/arithmetic/microunits.h"

namespace industry {
namespace decisions {
namespace {

constexpr market::Measure kMinPracticalScale = micro::kOneInU / 10;

} // namespace

void LocalProfitMaximiser::SelectCandidate(
    const ProductionContext& context,
    std::vector<proto::ProductionInfo>& candidates,
    proto::ProductionDecision* decision) const {
  market::Measure max_profit_u = 0;
  for (auto& prod_info : candidates) {
    market::Measure scale_u = prod_info.max_scale_u();
    if (scale_u < kMinPracticalScale) {
      prod_info.set_reject_reason("Impractical scale $0", scale_u);
      decision->add_rejected()->Swap(&prod_info);
      continue;
    }

    market::Measure reven_u = context.market->GetPriceU(
        prod_info.expected_output(), prod_info.step_info_size());
    market::Measure cost_u = 0;
    market::Measure current_scale_u = scale_u;
    for (int step = 0; step < prod_info.step_info_size(); ++step) {
      const industry::decisions::proto::StepInfo& step_info =
          prod_info.step_info(step);
      market::Measure lowest_var_cost_u = micro::kMaxU;
      market::Measure best_scale_u = current_scale_u;
      for (int var = 0; var < step_info.variant_size(); ++var) {
        const industry::decisions::proto::VariantInfo& var_info = step_info.variant(var);
        // TODO: Account for capital cost here - note that this is not a unit cost!
        // So this will require some thinking; how to account for existing capital?
        market::Measure var_cost_u = micro::MultiplyU(var_info.unit_cost_u(),
                                                      var_info.possible_scale_u());
        market::Measure var_scale_u = var_info.possible_scale_u();
        market::Measure scale_loss_u = current_scale_u - var_scale_u;
        if (scale_loss_u > 0) {
          var_cost_u += micro::MultiplyU(
              scale_loss_u, micro::DivideU(reven_u, prod_info.max_scale_u()));
        }
        if (var_cost_u < lowest_var_cost_u) {
          lowest_var_cost_u = var_cost_u;
          best_scale_u = var_scale_u;
        }
      }
      cost_u += lowest_var_cost_u;
      if (best_scale_u < current_scale_u) {
        current_scale_u = best_scale_u;
      }
    }

    market::Measure profit_u = reven_u - cost_u;
    if (profit_u <= 0) {
      prod_info.set_reject_reason(absl::Substitute(
          "Unprofitable, revenue $0 vs cost $1", reven_u, cost_u));
      decision->add_rejected()->Swap(&prod_info);
      continue;
    }

    if (profit_u <= max_profit_u) {
      prod_info.set_reject_reason(
          absl::Substitute("Less profit ($0) than $1 ($2)", profit_u,
                           decision->selected().name(), max_profit_u));
      decision->add_rejected()->Swap(&prod_info);
      continue;
    }
    max_profit_u = profit_u;
    if (decision->has_selected()) {
      decision->mutable_selected()->set_reject_reason(
          absl::Substitute("Less profit than $0", prod_info.name()));
      decision->add_rejected()->Swap(decision->mutable_selected());
    }
    decision->mutable_selected()->Swap(&prod_info);
  }
}

} // namespace decisions
} // namespace industry
