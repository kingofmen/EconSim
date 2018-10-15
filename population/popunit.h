// Class to represent a unit of population.
#ifndef BASE_POPULATION_POPUNIT_H
#define BASE_POPULATION_POPUNIT_H

#include <list>
#include <unordered_map>
#include <vector>

#include "geography/proto/geography.pb.h"
#include "industry/decisions/production_evaluator.h"
#include "industry/industry.h"
#include "industry/proto/industry.pb.h"
#include "market/market.h"
#include "market/proto/goods.pb.h"
#include "population/proto/population.pb.h"
#include "util/headers/int_types.h"

namespace population {

class PopUnit {
public:
  PopUnit();
  PopUnit(const proto::PopUnit& proto);

  // Add to lowest age group and subtract from oldest.
  void BirthAndDeath();

  void AutoProduce(const std::vector<const proto::AutoProduction*>& production,
                   market::Market* market);

  // Looks for packages in level that are possible to consume, either from
  // current resources or by buying from the market (which must not be null),
  // and returns the cheapest one in terms of the current market prices, or null
  // if there is none. The pointer cheapest is set to point to the cheapest
  // package whether or not it is currently achievable.
  const proto::ConsumptionPackage*
  CheapestPackage(const proto::ConsumptionLevel& level,
                  const market::Market& market,
                  const proto::ConsumptionPackage*& cheapest) const;

  // Attempts to consume the provided level, if necessary buying resources from
  // the market, which must not be null. Returns true on success.
  bool Consume(const proto::ConsumptionLevel& level, market::Market* market);

  // End-of-turn cleanup.
  void EndTurn(const market::proto::Container& decay_rates);

  int GetSize() const;

  // Attempt to continue existing production chains and start new ones. Returns
  // true if any chain makes progress.
  bool Produce(const industry::decisions::ProductionContext& context,
               industry::decisions::DecisionMap* production_info_map);

  // Start-of-turn setup. Finds the cheapest packages within levels to add up to
  // one unit of 'subsistence', and reserves that amount for internal use.
  void StartTurn(const std::vector<const proto::ConsumptionLevel*>& levels,
                 market::Market* market);

  static PopUnit* GetPopId(uint64 id) { return id_to_pop_map_[id]; }

  static uint64 NewPopId();

  // Sells what the pop won't use for subsistence.
  void SellSurplus(market::Market* market);

  market::proto::Container* mutable_wealth() { return proto_.mutable_wealth(); }
  const market::proto::Container& wealth() { return proto_.wealth(); }

  void ReturnCapital(market::proto::Container* caps);

  proto::PopUnit* Proto() { return &proto_; }

  uint64 pop_id() const { return proto_.pop_id(); }

private:
  static std::unordered_map<uint64, PopUnit*> id_to_pop_map_;

  // The underlying data in wire format.
  proto::PopUnit proto_;

  // Tracking how many consumption packages have been ordered from the market.
  int packages_ordered_;

  // The resources required for minimal subsistence.
  market::proto::Container subsistence_need_;

  // Resources that have been used as movable capital.
  market::proto::Container used_capital_;
};

} // namespace population

#endif
