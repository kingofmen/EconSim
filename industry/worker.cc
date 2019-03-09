#include "industry/worker.h"

#include <iostream>

#include "market/goods_utils.h"
#include "util/arithmetic/microunits.h"

namespace industry {
namespace {

// Returns the unit cost of input, with prices estimated by market at
// stepsAhead.
market::Measure GetUnitCostU(const proto::Input& input,
                             const market::PriceEstimator& market,
                             unsigned int stepsAhead) {
  return market.GetPriceU(input.consumables() + input.movable_capital(),
                          stepsAhead);
}

// Returns the unit cost of capex, with prices estimated by market at
// stepsAhead.
market::Measure GetCapCostU(const proto::Input& input,
                            const market::PriceEstimator& market,
                            unsigned int stepsAhead,
                            const market::proto::Container& fixcap) {
  auto required = market::SubtractFloor(input.fixed_capital(), fixcap);
  return market.GetPriceU(required, stepsAhead);
}

// Returns the maximum scale.
market::Measure GetMaxScaleU(const proto::Input& input,
                             const market::proto::Container& wealth,
                             const market::proto::Container& resources,
                             const market::proto::Container& fixed_capital,
                             const market::AvailabilityEstimator& market,
                             unsigned int stepsAhead, std::string* bottleneck) {
  market::Measure scale_u = micro::kMaxU;
  std::string bottle;
  // Look for resource bottlenecks.
  for (const auto& good : input.raw_materials().quantities()) {
    market::Measure ratio_u =
        micro::DivideU(market::GetAmount(resources, good.first), good.second);
    if (ratio_u < scale_u) {
      scale_u = ratio_u;
      bottle = good.first;
    }
  }

  // Now consumables bottlenecks.
  auto required = input.consumables() + input.movable_capital();
  for (const auto& good : required.quantities()) {
    market::Measure ratio_u =
        micro::DivideU((market.Available(good.first, stepsAhead) +
                        market::GetAmount(wealth, good.first)),
                       good.second);
    if (ratio_u < scale_u) {
      scale_u = ratio_u;
      bottle = good.first;
    }
  }

  // Finally fixcap. TODO: Account for install costs.
  const auto& fixcap = input.fixed_capital();
  for (const auto& good : fixcap.quantities()) {
    market::Measure ratio_u =
        micro::DivideU((market.Available(good.first, stepsAhead) +
                        market::GetAmount(fixed_capital, good.first) +
                        market::GetAmount(wealth, good.first)),
                       good.second);
    if (ratio_u < scale_u) {
      scale_u = ratio_u;
      bottle = good.first;
    }
  }

  if (bottleneck != nullptr && !bottle.empty()) {
    *bottleneck = bottle;
  }
  return scale_u;
}



void
FillProductionCostInfo(const Production& chain,
                       const market::PriceEstimator& prices,
                       const geography::proto::Field& field,
                       decisions::proto::ProductionInfo* production_info) {
  auto max_scale_u = chain.MaxScaleU();

  proto::Progress progress;
  if (field.has_progress()) {
    progress = field.progress();
  } else {
    progress = chain.MakeProgress(production_info->max_scale_u());
  }

  for (unsigned int step = progress.step(); step < chain.num_steps(); ++step) {
    auto ahead = step - progress.step();
    auto* step_info = production_info->add_step_info();
    const auto& prod_step = chain.get_step(step);
    for (unsigned int var = 0; var < prod_step.variants_size(); ++var) {
      const auto& input = prod_step.variants(var);
      auto* var_info = step_info->add_variant();
      var_info->set_unit_cost_u(GetUnitCostU(input, prices, ahead));
      var_info->set_cap_cost_u(
          GetCapCostU(input, prices, ahead, field.fixed_capital()));
    }
  }
}

// For each production chain in context, return a ProductionInfo with scale
// and unit cost if it passes all filters.
std::vector<decisions::proto::ProductionInfo>
GenerateOptions(const geography::proto::Field& field,
                const decisions::ProductionEvaluator& evaluator,
                const market::proto::Container& wealth,
                const decisions::ProductionContext& context,
                const std::vector<ProductionFilter*> filters) {
  std::vector<decisions::proto::ProductionInfo> ret;
  for (const auto& production : context.production_map) {
    bool pass = true;
    const Production* prod = production.second;
    for (const auto* filter : filters) {
      if (filter->Filter(field, *prod)) {
        continue;
      }
      pass = false;
      break;
    }
    if (!pass) {
      continue;
    }
    ret.push_back(
        evaluator.GetProductionInfo(*prod, wealth, *context.market, field));
  }
  return ret;
}

}  // namespace

void SelectProduction(const decisions::ProductionContext& context,
                      const market::proto::Container& wealth,
                      const std::vector<ProductionFilter*> filters,
                      const decisions::ProductionEvaluator& evaluator,
                      decisions::DecisionMap* info_map) {
  std::unordered_map<geography::proto::Field*,
                     std::vector<decisions::proto::ProductionInfo>>
      candidates;
  for (auto* field : context.fields) {
    candidates[field] = GenerateOptions(*field, evaluator, wealth, context, filters);
  }

  for (auto& cand : candidates) {
    if (cand.second.empty()) {
      continue;
    }
    decisions::proto::ProductionDecision decision;
    evaluator.SelectCandidate(context, cand.second, &decision);
    if (info_map->find(cand.first) != info_map->end()) {
      info_map->at(cand.first).Swap(&decision);
    } else {
      info_map->insert({cand.first, std::move(decision)});
    }
  }
}

// TODO: Don't pass the field here, pass the fixcap and resources. Clear
// separately.
bool TryProductionStep(
    const Production& production, const decisions::proto::StepInfo& step_info,
    geography::proto::Field* field, proto::Progress* progress,
    market::proto::Container* source, market::proto::Container* target,
    market::proto::Container* used_capital, market::Market* market) {

  auto variant_index = step_info.best_variant();
  if (variant_index >= step_info.variant_size()) {
    return false;
  }
  const auto& variant_info = step_info.variant(variant_index);
  if (progress->scaling_u() > variant_info.possible_scale_u()) {
    progress->set_scaling_u(variant_info.possible_scale_u());
  }

  auto required = production.RequiredConsumables(*progress, variant_index);
  required -= *source;
  if (!market->BuyBasket(required, source)) {
    return false;
  }

  production.PerformStep(field->fixed_capital(), 0, variant_index, source,
                         field->mutable_resources(), target, used_capital,
                         progress);

  if (production.Complete(*progress)) {
    field->clear_progress();
  }

  return true;
}

}  // namespace industry
