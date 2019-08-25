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

const std::vector<std::unique_ptr<proto::ProductionInfo>>&
getCands(ProductionContext* context, geography::proto::Field* field) {
  auto& cands = context->candidates.find(field);
  if (cands == context->candidates.end()) {
    static std::vector<std::unique_ptr<proto::ProductionInfo>> dummy;
    return dummy;
  }
  return cands->second;
}

proto::ProductionDecision* getDecision(ProductionContext* context, geography::proto::Field* field) {
  if (context->decisions->find(field) == context->decisions->end()) {
    (*context->decisions)[field] = proto::ProductionDecision();
  }
  return &(*context->decisions)[field];
}

void LocalProfitMaximiser::SelectCandidate(
    ProductionContext* context, geography::proto::Field* field) const {
  market::Measure max_profit_u = 0;
  proto::ProductionDecision* decision = getDecision(context, field);
  decision->Clear();
  const auto& candidates = getCands(context, field);
  if (candidates.empty()) {
    // This should never happen.
    return;
  }
  
  for (const auto& prod_info : candidates) {
    market::Measure scale_u = prod_info->max_scale_u();
    auto* reject = decision->add_rejected();
    *reject = *prod_info;
    if (scale_u < kMinPracticalScale) {
      reject->set_reject_reason(absl::Substitute("Impractical scale $0", scale_u));
      continue;
    }

    market::Measure reven_u = context->market->GetPriceU(
        prod_info->expected_output(), prod_info->step_info_size());
    market::Measure cost_u = 0;
    market::Measure current_scale_u = scale_u;
    for (int step = 0; step < prod_info->step_info_size(); ++step) {
      industry::decisions::proto::StepInfo* step_info =
          reject->mutable_step_info(step);
      market::Measure lowest_var_cost_u = micro::kMaxU;
      market::Measure best_scale_u = current_scale_u;
      int best_var = 0;
      for (int var = 0; var < step_info->variant_size(); ++var) {
        const industry::decisions::proto::VariantInfo& var_info =
            step_info->variant(var);
        market::Measure var_cost_u = micro::MultiplyU(
            var_info.unit_cost_u(), var_info.possible_scale_u());
        // Capital cost is not calculated as a unit, as that would not account
        // for existing capital; instead it is the cost-at-scale.
        var_cost_u += var_info.cap_cost_u();

        market::Measure var_scale_u = var_info.possible_scale_u();
        market::Measure scale_loss_u = current_scale_u - var_scale_u;
        if (scale_loss_u > 0) {
          var_cost_u += micro::MultiplyU(
              scale_loss_u, micro::DivideU(reven_u, prod_info->max_scale_u()));
        }
        if (var_cost_u < lowest_var_cost_u) {
          lowest_var_cost_u = var_cost_u;
          best_scale_u = var_scale_u;
          best_var = var;
        }
      }
      cost_u += lowest_var_cost_u;
      if (best_scale_u < current_scale_u) {
        current_scale_u = best_scale_u;
      }
      step_info->set_best_variant(best_var);
    }

    market::Measure profit_u = reven_u - cost_u;
    if (profit_u <= 0) {
      reject->set_reject_reason(absl::Substitute(
          "Unprofitable, revenue $0 vs cost $1", reven_u, cost_u));
      continue;
    }

    if (profit_u <= max_profit_u) {
      reject->set_reject_reason(
          absl::Substitute("Less profit ($0) than $1 ($2)", profit_u,
                           decision->selected().name(), max_profit_u));
      continue;
    }

    max_profit_u = profit_u;
    if (decision->has_selected()) {
      decision->mutable_selected()->set_reject_reason(
          absl::Substitute("Less profit than $0", prod_info->name()));
      reject->Swap(decision->mutable_selected());
      continue;
    }
    reject->Swap(decision->mutable_selected());
    reject = decision->mutable_rejected()->ReleaseLast();
    delete reject;
  }
}

} // namespace decisions
} // namespace industry
