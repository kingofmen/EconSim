#include "industry/decisions/production_evaluator.h"

#include <string>

#include "absl/strings/substitute.h"
#include "geography/geography.h"
#include "industry/industry.h"
#include "market/goods_utils.h"
#include "market/market.h"
#include "util/arithmetic/microunits.h"

namespace industry {
namespace decisions {
namespace {

unsigned int GetVariantIndex(const industry::Production& production,
                             const market::proto::Container& wealth,
                             const industry::proto::Progress& progress,
                             const market::Market& market,
                             const proto::StepInfo& step_info,
                             market::Measure* scale_u) {
  unsigned int variant_index = step_info.variant_size();
  market::Measure max_profit_u = 0;
  market::Measure max_money_u = market.MaxMoney(wealth);
  for (unsigned int idx = 0; idx < step_info.variant_size(); ++idx) {
    const auto& variant_info = step_info.variant(idx);
    if (variant_info.possible_scale_u() == 0) {
      continue;
    }
    market::Measure cost_at_scale_u = micro::MultiplyU(
        variant_info.unit_cost_u(), variant_info.possible_scale_u());
    market::Measure max_scale_u =
        std::min(variant_info.possible_scale_u(),
                 micro::DivideU(max_money_u, cost_at_scale_u));
    if (max_scale_u < 100000) {
      continue;
    }
    market::Measure profit_u =
        market.GetPriceU(production.ExpectedOutput(progress));
    profit_u -=
        micro::MultiplyU(variant_info.unit_cost_u(), progress.scaling_u());
    if (profit_u > max_profit_u) {
      max_profit_u = profit_u;
      variant_index = idx;
      *scale_u = max_scale_u;
    }
  }
  return variant_index;
}

void GetStepInfo(const industry::Production& production,
                 const market::proto::Container& wealth,
                 const market::Market& market,
                 const geography::proto::Field& field,
                 const industry::proto::Progress& progress,
                 proto::StepInfo* step_info) {
  const auto& step = production.Proto()->steps(progress.step());
  for (int index = 0; index < step.variants_size(); ++index) {
    auto* variant_info = step_info->add_variant();
    const auto& input = step.variants(index);
    if (!(field.fixed_capital() > input.fixed_capital())) {
      continue;
    }
    variant_info->set_possible_scale_u(progress.scaling_u());
    for (const auto& good : input.raw_materials().quantities()) {
      market::Measure ratio_u = micro::DivideU(
          micro::kOneInU * market::GetAmount(field.resources(), good.first),
          micro::kOneInU * good.second);

      if (ratio_u < variant_info->possible_scale_u()) {
        variant_info->set_possible_scale_u(ratio_u);
      }
    }

    auto required = production.RequiredConsumables(progress.step(), index);
    for (const auto& good : required.quantities()) {
      auto unit_cost_u = variant_info->unit_cost_u();
      unit_cost_u +=
          micro::MultiplyU(market.GetPriceU(good.first), good.second);
      variant_info->set_unit_cost_u(unit_cost_u);
      market::Measure ratio_u =
          micro::DivideU((market.AvailableImmediately(good.first) +
                          market::GetAmount(wealth, good.first)),
                         good.second);
      if (ratio_u < variant_info->possible_scale_u()) {
        variant_info->set_possible_scale_u(ratio_u);
      }
    }
  }
}

} // namespace

proto::ProductionInfo ProductionEvaluator::GetProductionInfo(
    const industry::Production& chain, const market::proto::Container& wealth,
    const market::Market& market, const geography::proto::Field& field) const {
  proto::ProductionInfo ret;
  ret.set_max_scale_u(chain.MaxScaleU());

  int current_step = 0;
  industry::proto::Progress progress;
  if (field.has_progress()) {
    progress = field.progress();
  } else {
    progress = chain.MakeProgress(ret.max_scale_u());
  }

  for (int i = progress.step(); i < chain.Proto()->steps_size(); ++i) {
    progress.set_step(i);
    auto* step_info = ret.add_step_info();
    GetStepInfo(chain, wealth, market, field, progress, step_info);
    market::Measure scale_u = 0;
    step_info->set_best_variant(
        GetVariantIndex(chain, wealth, progress, market, *step_info, &scale_u));
    if (step_info->best_variant() >= step_info->variant_size()) {
      ret.set_max_scale_u(0);
      return ret;
    }
    if (scale_u < ret.max_scale_u()) {
      ret.set_max_scale_u(scale_u);
    }
    auto total_unit_cost_u = ret.total_unit_cost_u();
    total_unit_cost_u +=
        step_info->variant(step_info->best_variant()).unit_cost_u();
    ret.set_total_unit_cost_u(total_unit_cost_u);
  }
  return ret;
}

proto::ProductionDecision
LocalProfitMaximiser::Evaluate(const ProductionContext& context,
                               const market::proto::Container& wealth,
                               geography::proto::Field* target) const {
  proto::ProductionDecision ret;
  std::unordered_map<std::string, proto::ProductionInfo> possible_chains;
  for (const auto& chain : context.production_map) {
    const auto* production = chain.second;
    if (!geography::HasLandType(*target, *production)) {
      ret.mutable_insufficient()->insert({chain.first, "Wrong land type"});
      continue;
    }
    if (!geography::HasFixedCapital(*target, *production)) {
      ret.mutable_insufficient()->insert(
          {chain.first, "Not enough fixed capital"});
      continue;
    }
    if (!geography::HasRawMaterials(*target, *production)) {
      ret.mutable_insufficient()->insert(
          {chain.first, "Not enough raw material"});
      continue;
    }
    possible_chains.emplace(
        chain.first,
        GetProductionInfo(*production, wealth, *context.market, *target));
  }
  if (possible_chains.empty()) {
    return ret;
  }

  market::Measure max_profit_u = 0;
  for (auto& possible : possible_chains) {
    const auto* chain = context.production_map.at(possible.first);
    auto& info = possible.second;
    info.set_name(possible.first);
    market::Measure profit_u =
        context.market->GetPriceU(chain->Proto()->outputs());
    profit_u -= info.total_unit_cost_u();
    if (profit_u <= 0) {
      info.set_reject_reason("Unprofitable");
      ret.add_rejected()->Swap(&info);
      continue;
    }
    if (info.max_scale_u() <= 0) {
      info.set_reject_reason("Impractical");
      ret.add_rejected()->Swap(&info);
      continue;
    }
    profit_u = micro::MultiplyU(profit_u, info.max_scale_u());
    if (profit_u <= max_profit_u) {
      info.set_reject_reason(
          absl::Substitute("Less profit than $0", ret.selected().name()));
      ret.add_rejected()->Swap(&info);
      continue;
    }
    max_profit_u = profit_u;
    ret.mutable_selected()->set_reject_reason(
        absl::Substitute("Less profit than $0", info.name()));
    ret.mutable_selected()->Swap(&info);
  }

  return ret;
}

} // namespace decisions
} // namespace industry
