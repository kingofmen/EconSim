#include "industry/worker.h"

#include <iostream>

#include "market/goods_utils.h"

namespace industry {
namespace {

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
    ret.push_back(evaluator.GetProductionInfo(*prod, wealth, *context.market, field));
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
    std::cout << "Decision: " << decision.DebugString() << "\n";
    if (info_map->find(cand.first) != info_map->end()) {
      info_map->at(cand.first).Swap(&decision);
    } else {
      info_map->insert({cand.first, std::move(decision)});
    }
  }
}

// TODO: Don't pass the field here, pass the fixcap and resources. Clear
// separately.
// TODO: Just pass the StepInfo?
bool TryProductionStep(
    const industry::Production& production,
    const industry::decisions::proto::StepInfo& step_info,
    geography::proto::Field* field, industry::proto::Progress* progress,
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
