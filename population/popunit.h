// Class to represent a unit of population.
#ifndef POPULATION_POPUNIT_H
#define POPULATION_POPUNIT_H

#include <unordered_map>

#include "market/goods_utils.h"
#include "market/proto/goods.pb.h"
#include "population/proto/population.pb.h"

using google::protobuf::uint64;

namespace population {

class PopUnit : public proto::PopUnit {
public:
  PopUnit();

  // Add to lowest age group and subtract from oldest.
  void BirthAndDeath();

  void AutoProduce(const std::vector<proto::AutoProduction*>& production,
                   const market::proto::Container& prices);

  const proto::ConsumptionPackage*
  CheapestPackage(const proto::ConsumptionLevel& level,
                  const market::proto::Container& prices) const;

  // Turn resources into consumption levels. Returns true if the POP can
  // consume at this level.
  bool Consume(const proto::ConsumptionLevel& level,
               const market::proto::Container& prices);

  int GetSize() const;

  static PopUnit* GetPopId(uint64 id) { return id_to_pop_map_[id]; }

  static uint64 NewPopId();

private:
  static std::unordered_map<uint64, PopUnit*> id_to_pop_map_;
};

} // namespace population

#endif
