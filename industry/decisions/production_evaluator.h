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
typedef std::unordered_map<geography::proto::Field*, proto::ProductionDecision>
    DecisionMap;

struct ProductionContext {
  ProductionMap production_map;
  std::unordered_set<geography::proto::Field*> fields;
  market::Market* market;
};

class ProductionEvaluator {
public:
  // Fills in the 'selected' and 'rejected' fields of decision. Assumes all
  // candidates are possible.
  virtual void SelectCandidate(const ProductionContext& context,
                               std::vector<proto::ProductionInfo>& candidates,
                               proto::ProductionDecision* decision) const = 0;
};

// Evaluates the profit the chain will make, assuming all inputs can be bought
// and all outputs sold at the quoted market prices. Ignores the context.
class LocalProfitMaximiser : public ProductionEvaluator {
public:
  void SelectCandidate(const ProductionContext& context,
                       std::vector<proto::ProductionInfo>& candidates,
                       proto::ProductionDecision* decision) const override;
};

}  // namespace decisions
}  // namespace industry

#endif
