// Defines an interface for evaluating candidate production chains.
#ifndef POPULATION_PRODUCTION_EVALUATOR
#define POPULATION_PRODUCTION_EVALUATOR

#include "geography/proto/geography.pb.h"
#include "industry/industry.h"

namespace population {
  typedef std::unordered_map<std::string, const industry::Production*>
      ProductionMap;

struct ProductionContext {
  ProductionMap production_map;
  std::vector<geography::proto::Field*> fields;
  market::Market* market;
};

class ProductionEvaluator {
 public:
   virtual double Evaluate(const ProductionContext& context,
                           geography::proto::Field* target) const = 0;
};

// Evaluates the profit the chain will make, assuming all inputs can be bought
// and all outputs sold at the quoted market prices. Ignores the context.
class LocalProfitMaximiser : public ProductionEvaluator {
 public:
   double Evaluate(const ProductionContext& context,
                   geography::proto::Field* target) const override;
};

}  // namespace population

#endif
