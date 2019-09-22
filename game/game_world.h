// Class to hold the world and simulate time steps.
#ifndef GAME_GAME_WORLD_HH
#define GAME_GAME_WORLD_HH

#include <memory>
#include <vector>

#include "factions/factions.h"
#include "factions/proto/factions.pb.h"
#include "game/proto/game_world.pb.h"
#include "geography/connection.h"
#include "geography/geography.h"
#include "geography/proto/geography.pb.h"
#include "industry/decisions/production_evaluator.h"
#include "industry/proto/industry.pb.h"
#include "population/popunit.h"
#include "population/proto/population.pb.h"
#include "units/unit.h"

namespace game {

class GameWorld {
public:
  GameWorld(const proto::GameWorld& world, proto::Scenario* scenario);
  ~GameWorld();

  struct Scenario {
    Scenario(proto::Scenario* scenario);
    std::vector<const population::proto::AutoProduction*> auto_production_;
    std::vector<const industry::proto::Production*> production_chains_;
    std::vector<const population::proto::ConsumptionLevel*> subsistence_;
    market::proto::Container decay_rates_;
    proto::Scenario proto_;
  };

  // Sets the production evaluator for the field.
  void SetProductionEvaluator(uint64 area_id, uint64 field_idx,
                              industry::decisions::ProductionEvaluator* eval);

  // Moves the simulation forward one step.
  void TimeStep(industry::decisions::FieldMap<
                industry::decisions::proto::ProductionDecision>*
                    production_decisions);

  // Copies the current game state (not scenario) into the proto, which must not
  // be null.
  void SaveToProto(proto::GameWorld* proto) const;

  // Returns the names of the known production chains.
  const std::vector<std::string>& chain_names() const { return chain_names_; }

  // Returns the chain.
  // TODO: Error handling on a bad name being provided.
  const industry::Production& chain(const std::string& name) const {
    return *(production_map_.at(name));
  }

private:
  // 'Setup' information that does not change in the simulation.
  Scenario scenario_;

  // World-state information.
  std::vector<std::unique_ptr<factions::FactionController>> factions_;
  std::vector<std::unique_ptr<population::PopUnit>> pops_;
  std::vector<std::unique_ptr<geography::Area>> areas_;
  std::vector<std::unique_ptr<geography::Connection>> connections_;
  std::vector<std::unique_ptr<units::Unit>> units_;
  std::unordered_map<std::string, const industry::Production*> production_map_;
  industry::decisions::ProductionEvaluator* default_evaluator_;

  // Player information.
  std::unordered_map<geography::proto::Field*,
                     industry::decisions::ProductionEvaluator*>
      production_evaluators_;

  // Cached scenario information.
  std::vector<std::string> chain_names_;
};

} // namespace game

#endif
