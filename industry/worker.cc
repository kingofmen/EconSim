#include "industry/worker.h"

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
  bool pass = true;
  for (const auto& production : context.production_map) {
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
    info_map->insert({cand.first, std::move(decision)});
  }
}

}  // namespace industry
