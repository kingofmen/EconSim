// Class to hold the world and simulate time steps.
#ifndef GAME_GAME_WORLD_HH
#define GAME_GAME_WORLD_HH

#include <memory>
#include <vector>

#include "game/proto/game_world.pb.h"
#include "geography/geography.h"
#include "geography/proto/geography.pb.h"
#include "industry/proto/industry.pb.h"
#include "population/popunit.h"
#include "population/proto/population.pb.h"

namespace game {

class GameWorld {
public:
  GameWorld(const proto::GameWorld& world, proto::Scenario* scenario);

  struct Scenario {
    Scenario(proto::Scenario* scenario);
    std::vector<const population::proto::AutoProduction*> auto_production_;
    std::vector<const industry::proto::Production*> production_chains_;
    proto::Scenario proto_;
  };

  void TimeStep();

  // Copies the current game state (not scenario) into the proto, which must not
  // be null.
  void SaveToProto(proto::GameWorld* proto) const;

private:
  // 'Setup' information that does not change in the simulation.
  Scenario scenario_;

  // World-state information.
  std::vector<std::unique_ptr<population::PopUnit>> pops_;
  std::vector<std::unique_ptr<geography::Area>> areas_;
  population::PopUnit::ProductionMap production_map_;
};

} // namespace game

#endif
