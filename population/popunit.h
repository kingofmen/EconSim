// Class to represent a unit of population.
#ifndef BASE_POPULATION_POPUNIT_H
#define BASE_POPULATION_POPUNIT_H

#include <list>
#include <unordered_map>
#include <vector>

#include "geography/proto/geography.pb.h"
#include "industry/industry.h"
#include "industry/proto/industry.pb.h"
#include "market/market.h"
#include "market/goods_utils.h"
#include "market/proto/goods.pb.h"
#include "population/production_evaluator.h"
#include "population/proto/population.pb.h"

namespace population {

using google::protobuf::uint64;

class PopUnit {
public:
  PopUnit();
  PopUnit(const proto::PopUnit& proto);

  // Add to lowest age group and subtract from oldest.
  void BirthAndDeath();

  void AutoProduce(const std::vector<const proto::AutoProduction*>& production,
                   const market::proto::Container& prices);

  const proto::ConsumptionPackage*
  CheapestPackage(const proto::ConsumptionLevel& level,
                  const market::proto::Container& prices) const;

  // Turn resources into consumption levels. Returns true if the POP can
  // consume at this level.
  bool Consume(const proto::ConsumptionLevel& level,
               const market::proto::Container& prices);

  // End-of-turn cleanup.
  void EndTurn(const market::proto::Container& decay_rates);

  int GetSize() const;

  // Attempt to continue existing production chains and start new ones. Returns
  // true if any chain makes progress.
  bool Produce(const ProductionContext& context);

  // Attempts to find a new production chain to run in field.
  bool StartNewProduction(const ProductionContext& context,
                          geography::proto::Field* field);

  // Attempts to run the next step of production. Returns true if the process
  // advances.
  bool TryProductionStep(const industry::Production& production,
                         const proto::ProductionInfo& production_info,
                         geography::proto::Field* field,
                         industry::proto::Progress* progress,
                         market::Market* market);

  static PopUnit* GetPopId(uint64 id) { return id_to_pop_map_[id]; }

  static uint64 NewPopId();

  proto::PopUnit* Proto() {return &proto_;}

  void set_production_evaluator(ProductionEvaluator* eval) {
    evaluator_ = eval;
  }

 private:
  static std::unordered_map<uint64, PopUnit*> id_to_pop_map_;

  static ProductionEvaluator& default_evaluator_;

  // Keeps track of process information for the turn.
  std::unordered_set<geography::proto::Field*> fields_worked_;
  std::unordered_map<geography::proto::Field*, proto::ProductionInfo>
      production_info_map_;

  proto::PopUnit proto_;

  // For choosing new production chains. Not owned.
  ProductionEvaluator* evaluator_;
};

} // namespace population

#endif
