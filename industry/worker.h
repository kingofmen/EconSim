// Interfaces and methods for selecting and executing Production objects.
#ifndef BASE_INDUSTRY_WORKER_H
#define BASE_INDUSTRY_WORKER_H

#include "geography/proto/geography.pb.h"
#include "industry/decisions/production_evaluator.h"
#include "industry/industry.h"
#include "market/market.h"
#include "market/proto/goods.pb.h"

namespace industry {

// Interface allowing filtering of production chains.
class ProductionFilter {
 public:
  // Returns true if the field can run the production.
  virtual bool Filter(const geography::proto::Field&, const Production&) const = 0;
};

// Calculate the maximum scale of each variant of each production candidate from
// the availability of resources.
void CalculateProductionScale(const market::proto::Container& wealth,
                              decisions::ProductionContext* context,
                              geography::proto::Field* field);

// Calculates unit costs, including capex, for each step and variant of chain.
void CalculateProductionCosts(
    const industry::Production& chain, const market::PriceEstimator& prices,
    const geography::proto::Field& field,
    industry::decisions::proto::ProductionInfo* production_info);

// Uses evaluator to select a production chain for the field.
void SelectProduction(const decisions::ProductionEvaluator& evaluator,
                      decisions::ProductionContext* context,
                      geography::proto::Field* field);

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
