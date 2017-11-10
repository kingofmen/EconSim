#include "industry/decisions/production_evaluator.h"

#include <string>

#include "absl/strings/substitute.h"
#include "market/goods_utils.h"
#include "market/market.h"
#include "geography/geography.h"
#include "industry/industry.h"

namespace industry {
namespace decisions {
namespace {

unsigned int GetVariantIndex(const industry::Production& production,
                             const market::proto::Container& wealth,
                             const industry::proto::Progress& progress,
                             const market::Market& market,
                             const proto::StepInfo& step_info,
                             double* scale) {
  unsigned int variant_index = step_info.variant_size();
  double max_profit = 0;
  double max_money = market.MaxMoney(wealth);
  for (unsigned int idx = 0; idx < step_info.variant_size(); ++idx) {
    const auto& variant_info = step_info.variant(idx);
    double cost_at_scale = variant_info.unit_cost() * variant_info.possible_scale();
    double max_scale =
        std::min(variant_info.possible_scale(), max_money / cost_at_scale);
    if (max_scale < 0.1) {
      continue;
    }
    double profit = market.GetPrice(production.ExpectedOutput(progress));
    profit -= variant_info.unit_cost() * progress.scaling();
    if (profit > max_profit) {
      max_profit = profit;
      variant_index = idx;
      *scale = max_scale;
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
    variant_info->set_possible_scale(progress.scaling());
    for (const auto& good : input.raw_materials().quantities()) {
      double ratio = market::GetAmount(field.resources(), good.first);
      ratio /= good.second;
      if (ratio < variant_info->possible_scale()) {
        variant_info->set_possible_scale(ratio);
      }
    }

    auto required = production.RequiredConsumables(progress.step(), index);
    for (const auto& good : required.quantities()) {
      auto unit_cost = variant_info->unit_cost();
      unit_cost += market.GetPrice(good.first) * good.second;
      variant_info->set_unit_cost(unit_cost);
      double ratio = market.AvailableImmediately(good.first) +
                     market::GetAmount(wealth, good.first);
      ratio /= good.second;
      if (ratio < variant_info->possible_scale()) {
        variant_info->set_possible_scale(ratio);
      }
    }
  }
}

}  // namespace

proto::ProductionInfo ProductionEvaluator::GetProductionInfo(
    const industry::Production& chain, const market::proto::Container& wealth,
    const market::Market& market, const geography::proto::Field& field) const {
  proto::ProductionInfo ret;
  ret.set_max_scale(chain.MaxScale());

  int current_step = 0;
  industry::proto::Progress progress;
  if (field.has_progress()) {
    progress = field.progress();
  } else {
    progress = chain.MakeProgress(ret.max_scale());
  }

  for (int i = progress.step(); i < chain.Proto()->steps_size(); ++i) {
    progress.set_step(i);
    auto* step_info = ret.add_step_info();
    GetStepInfo(chain, wealth, market, field, progress, step_info);
    double scale = 0;
    step_info->set_best_variant(
        GetVariantIndex(chain, wealth, progress, market, *step_info, &scale));
    if (step_info->best_variant() >= step_info->variant_size()) {
      ret.set_max_scale(0);
      return ret;
    }
    if (scale < ret.max_scale()) {
      ret.set_max_scale(scale);
    }
    auto total_unit_cost = ret.total_unit_cost();
    total_unit_cost +=
        step_info->variant(step_info->best_variant()).unit_cost();
    ret.set_total_unit_cost(total_unit_cost);
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
      ret.mutable_insufficient()->insert({chain.first, "Not enough fixed capital"});
      continue;
    }
    if (!geography::HasRawMaterials(*target, *production)) {
      ret.mutable_insufficient()->insert({chain.first, "Not enough raw material"});
      continue;
    }
    possible_chains.emplace(
        chain.first, GetProductionInfo(*production, wealth, *context.market, *target));
  }
  if (possible_chains.empty()) {
    return ret;
  }

  double max_profit = 0;
  for (auto& possible : possible_chains) {
    const auto* chain = context.production_map.at(possible.first);
    auto& info = possible.second;
    info.set_name(possible.first);
    double profit = context.market->GetPrice(chain->Proto()->outputs());
    profit -= info.total_unit_cost();
    if (profit <= 0) {
      info.set_reject_reason("Unprofitable");
      ret.add_rejected()->Swap(&info);
      continue;
    }
    profit *= info.max_scale();
    if (profit <= max_profit) {
      info.set_reject_reason(
          absl::Substitute("Less profit than $0", ret.selected().name()));
      ret.add_rejected()->Swap(&info);
      continue;
    }
    max_profit = profit;
    ret.mutable_selected()->set_reject_reason(
        absl::Substitute("Less profit than $0", info.name()));
    ret.mutable_selected()->Swap(&info);
  }

  return ret;
}

}  // namespace decisions
}  // namespace industry
