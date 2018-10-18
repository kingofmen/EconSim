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

// For each field in context, generates and filters options for production,
// calculates their possible scale, and uses evaluator to select the best one,
// storing the decisions and reasons for them in info_map.
void SelectProduction(const decisions::ProductionContext& context,
                      const market::proto::Container& wealth,
                      const std::vector<ProductionFilter*> filters,
                      const decisions::ProductionEvaluator& evaluator,
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
