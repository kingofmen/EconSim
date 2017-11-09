// Defines an interface for evaluating candidate production chains.
#ifndef POPULATION_PRODUCTION_EVALUATOR
#define POPULATION_PRODUCTION_EVALUATOR

#include "geography/proto/geography.pb.h"
#include "industry/industry.h"
#include "industry/proto/decisions.pb.h"
#include "market/proto/goods.pb.h"

namespace industry {
namespace decisions {
typedef std::unordered_map<std::string, const industry::Production*>
    ProductionMap;

struct ProductionContext {
  ProductionMap production_map;
  std::vector<geography::proto::Field*> fields;
  market::Market* market;
};

class ProductionEvaluator {
public:
  // Calculates the maximum scale and unit cost of the provided chain.
  proto::ProductionInfo GetProductionInfo(
      const industry::Production& chain, const market::proto::Container& wealth,
      const market::Market& market, const geography::proto::Field& field) const;

  virtual proto::ProductionDecision
  Evaluate(const ProductionContext& context,
           const market::proto::Container& wealth,
           geography::proto::Field* target) const = 0;
};

// Evaluates the profit the chain will make, assuming all inputs can be bought
// and all outputs sold at the quoted market prices. Ignores the context.
class LocalProfitMaximiser : public ProductionEvaluator {
public:
  proto::ProductionDecision
  Evaluate(const ProductionContext& context,
           const market::proto::Container& wealth,
           geography::proto::Field* target) const override;
};

}  // namespace decisions
}  // namespace industry

#endif
