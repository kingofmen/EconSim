// Defines an interface for evaluating candidate production chains.
#ifndef POPULATION_PRODUCTION_EVALUATOR
#define POPULATION_PRODUCTION_EVALUATOR

#include "geography/proto/geography.pb.h"
#include "industry/industry.h"
#include "industry/proto/decisions.pb.h"
#include "market/proto/goods.pb.h"

namespace industry {
namespace decisions {

template <class T>
using FieldMap = std::unordered_map<geography::proto::Field*, T>;

class ProductionEvaluator;

struct FieldInfo {
  ProductionEvaluator* evaluator;
  proto::ProductionDecision decision;
  std::vector<std::unique_ptr<proto::ProductionInfo>> candidates;
};

struct ProductionContext {
  const std::unordered_map<std::string, const industry::Production*>*
      production_map;
  FieldMap<FieldInfo> fields;
  market::Market* market;
};

class ProductionEvaluator {
public:
  // Fills in the 'selected' and 'rejected' fields of the decision for field in
  // the context DecisionMap.
  virtual void SelectCandidate(ProductionContext* context,
                               geography::proto::Field* field) const = 0;
};

// Evaluates the profit the chain will make, assuming all inputs can be bought
// and all outputs sold at the quoted market prices. Ignores the context.
class LocalProfitMaximiser : public ProductionEvaluator {
public:
  void SelectCandidate(ProductionContext* context,
                       geography::proto::Field* field) const override;
};

// Decides based on outside sources, e.g. player input; if there is none,
// falls back on an algorithmic evaluator.
class FieldSpecifier : public ProductionEvaluator {
public:
  FieldSpecifier(ProductionEvaluator* f) : fallback_(f) {}
  void SelectCandidate(ProductionContext* context,
                       geography::proto::Field* field) const override;
  void SetFieldProduction(const geography::proto::Field* field,
                          const std::string& chain);
  std::string
  CurrentFieldProduction(const geography::proto::Field* field) const;

private:
  ProductionEvaluator* fallback_;
  std::unordered_map<const geography::proto::Field*, std::string> chains_;
};

}  // namespace decisions
}  // namespace industry

#endif
