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
#include "population/proto/population.pb.h"

namespace population {

using google::protobuf::uint64;

class PopUnit : public proto::PopUnit {
public:
  typedef std::unordered_map<std::string, const industry::Production*>
      ProductionMap;

  struct VariantInfo {
    double unit_cost = 0;
    double possible_scale = 0;
  };

  struct ProductionStepInfo {
    int attempts_this_turn = 0;
    std::vector<VariantInfo> variants;
  };

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

  // Reduce wealth due to entropy.
  void DecayWealth(const market::proto::Container& decay_rates);

  int GetSize() const;

  // Calculates cost and possibility information for each variant of the current
  // step in progress, and stores it in step_info, which may not be null.
  void GetStepInfo(const industry::Production& production,
                   const market::Market& market,
                   const geography::proto::Field& field,
                   const industry::proto::Progress& progress,
                   ProductionStepInfo* step_info) const;

  // Attemp to continue existing production chains and start new ones. Returns
  // true if any chain makes progress.
  bool Produce(const ProductionMap& chains,
               const std::vector<geography::proto::Field*>& fields,
               market::Market* market);

  // Attempts to run the next step of production. Returns true if the process
  // advances.
  bool TryProductionStep(const industry::Production& production,
                         geography::proto::Field* field,
                         industry::proto::Progress* progress,
                         market::Market* market, ProductionStepInfo* step_info);

  static PopUnit* GetPopId(uint64 id) { return id_to_pop_map_[id]; }

  static uint64 NewPopId();

private:
  static std::unordered_map<uint64, PopUnit*> id_to_pop_map_;

  // Keeps track of which processes have progressed this turn.
  std::unordered_set<industry::proto::Progress*> progressed_;
};

} // namespace population

#endif
