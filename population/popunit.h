// Class to represent a unit of population.
#ifndef BASE_POPULATION_POPUNIT_H
#define BASE_POPULATION_POPUNIT_H

#include <unordered_map>
#include <vector>

#include "geography/proto/geography.pb.h"
#include "industry/industry.h"
#include "industry/proto/industry.pb.h"
#include "market/goods_utils.h"
#include "market/proto/goods.pb.h"
#include "population/proto/population.pb.h"

namespace population {

using google::protobuf::uint64;

class PopUnit : public proto::PopUnit {
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

  // Reduce wealth due to entropy.
  void DecayWealth(const market::proto::Container& decay_rates);

  int GetSize() const;

  // Choose which fields to continue or start production in.
  void Produce(const market::proto::Container& prices,
               const std::unordered_map<std::string,
                                        const industry::Production*>& chains,
               const std::vector<geography::proto::Field*>& fields);

  static PopUnit* GetPopId(uint64 id) { return id_to_pop_map_[id]; }

  static uint64 NewPopId();

private:
  // Struct for storing a possible production chain. Does not take ownership of
  // the pointers. The Progress pointer may be null, indicating that the
  // production has not started.
  struct ProductionCandidate {
    ProductionCandidate(geography::proto::Field* t,
                        const industry::Production* p,
                        const industry::proto::Progress* pr)
        : target(t), process(p), progress(pr) {}
    geography::proto::Field* target;
    const industry::Production* process;
    const industry::proto::Progress* progress;
    market::proto::Container heuristics;
  };

  // Calculates inputs into production-decision algorithm.
  void CalculateCandidateHeuristics(
      const market::proto::Container& prices,
      const std::unordered_map<std::string, const industry::Production*>&
          chains,
      const std::vector<ProductionCandidate>& selected,
      ProductionCandidate* candidate) const;

  static std::unordered_map<uint64, PopUnit*> id_to_pop_map_;
};

} // namespace population

#endif
