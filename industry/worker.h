// Interfaces and methods for selecting and executing Production objects.
#ifndef BASE_INDUSTRY_WORKER_H
#define BASE_INDUSTRY_WORKER_H

#include "geography/proto/geography.pb.h"
#include "industry/decisions/production_evaluator.h"
#include "industry/industry.h"
#include "market/market.h"
#include "market/proto/goods.pb.h"

namespace industry {

typedef std::unordered_map<
    geography::proto::Field*,
    std::vector<industry::decisions::proto::ProductionInfo>>
    FieldInfoMap;

// Interface allowing filtering of production chains.
class ProductionFilter {
 public:
  // Returns true if the field can run the production.
  virtual bool Filter(const geography::proto::Field&, const Production&) const = 0;
};

// Calculate the maximum scale of each variant of each production candidate in
// the field_info map from the availability of resources, ignoring their price.
void CalculateProductionScale(
    const industry::decisions::ProductionMap& production_map,
    const market::proto::Container& wealth, const market::Market& market,
    FieldInfoMap* field_info);

// Calculates unit costs, including capex, for each step and variant of chain.
void CalculateProductionCosts(
    const industry::Production& chain, const market::PriceEstimator& prices,
    const geography::proto::Field& field,
    industry::decisions::proto::ProductionInfo* production_info);

// For each field in context, uses evaluator to select one of the candidate
// production types stored in field_info, storing the decisions in info_map.
void SelectProduction(const decisions::ProductionContext& context,
                      const decisions::ProductionEvaluator& evaluator,
                      FieldInfoMap& field_info,
                      decisions::DecisionMap* info_map);

// Attempts to run the next step of production. Returns true if the process
// advances. Any output goods are put into target. No pointers may be null.
bool TryProductionStep(const industry::Production& production,
                       const industry::decisions::proto::StepInfo& step_info,
                       geography::proto::Field* field,
                       industry::proto::Progress* progress,
                       market::proto::Container* source,
                       market::proto::Container* target,
                       market::proto::Container* used_capital,
                       market::Market* market);
}  // namespace industry

#endif
