// Interfaces and methods for selecting and executing Production objects.
#ifndef BASE_INDUSTRY_WORKER_H
#define BASE_INDUSTRY_WORKER_H

#include "games/geography/proto/geography.pb.h"
#include "games/industry/decisions/production_evaluator.h"
#include "games/industry/industry.h"
#include "games/market/market.h"
#include "games/market/proto/goods.pb.h"
#include "util/status/status.h"

namespace industry {

// Interface allowing filtering of production chains.
class ProductionFilter {
 public:
  // Returns true if the field can run the production.
  virtual bool Filter(const geography::proto::Field&, const Production&) const = 0;
};

// Calculates the maximum scale of each variant of each production candidate
// from the availability of resources; also calculates the resulting capital
// costs.
void CalculateProductionScale(const market::proto::Container& wealth,
                              decisions::ProductionContext* context,
                              geography::proto::Field* field);

// Calculates unit costs for each step and variant of chain.
void CalculateProductionCosts(
    const industry::Production& chain, const market::PriceEstimator& prices,
    const geography::proto::Field& field,
    industry::decisions::proto::ProductionInfo* production_info);

// Uses evaluator to select a production chain for the field.
// DEPRECATED. Use the evaluator's SelectCandidate method directly instead.
void SelectProduction(const decisions::ProductionEvaluator& evaluator,
                      decisions::ProductionContext* context,
                      geography::proto::Field* field);

// Attempts to install the required fixed capital for the provided step of the
// chain. Returns true if all the capital can be installed; otherwise no changes
// are made.
bool InstallFixedCapital(const industry::proto::Input& production,
                         micro::Measure scale_u,
                         market::proto::Container* source,
                         market::proto::Container* target,
                         market::Market* market);

// Attempts to run the next step of production. Returns OK if the process
// advances. Any output goods are put into target. No pointers may be null.
util::Status TryProductionStep(
    const industry::Production& production,
    const industry::decisions::proto::StepInfo& step_info,
    geography::proto::Field* field, industry::proto::Progress* progress,
    market::proto::Container* source, market::proto::Container* target,
    market::proto::Container* used_capital, market::Market* market);

}  // namespace industry

#endif
