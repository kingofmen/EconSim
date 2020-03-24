#ifndef GAMES_SETUP_SETUP_H
#define GAMES_SETUP_SETUP_H

#include <memory>
#include <vector>

#include "factions/factions.h"
#include "factions/proto/factions.pb.h"
#include "games/setup/proto/setup.pb.h"
#include "geography/connection.h"
#include "geography/geography.h"
#include "geography/proto/geography.pb.h"
#include "population/popunit.h"
#include "population/proto/population.pb.h"
#include "units/unit.h"
#include "util/status/status.h"

namespace games {
namespace setup {

// Object to hold unchanging scenario information.
struct Constants {
  Constants(const games::setup::proto::Scenario& scenario);
  std::vector<const population::proto::AutoProduction*> auto_production_;
  std::vector<const industry::proto::Production*> production_chains_;
  market::proto::Container decay_rates_;
};

// Object to hold current state in memory.
class World {
 public:
  // TODO: Make this a StatusOr when Abseil releases that.
  // Create a World from the proto object.
  static std::unique_ptr<World> FromProto(const proto::GameWorld& proto);

  // Save state to proto, which must not be null.
  util::Status ToProto(proto::GameWorld* proto);

private:
  // World-state information.
  std::vector<std::unique_ptr<factions::FactionController>> factions_;
  std::vector<std::unique_ptr<population::PopUnit>> pops_;
  std::vector<std::unique_ptr<geography::Area>> areas_;
  std::vector<std::unique_ptr<geography::Connection>> connections_;
  std::vector<std::unique_ptr<units::Unit>> units_;
};

util::Status LoadScenario(const proto::ScenarioFiles& config,
                          proto::Scenario* scenario);

util::Status LoadWorld(const proto::ScenarioFiles& config,
                       proto::GameWorld* world);

// Load world and constants protos from config. Note that this
// resets the world pointer.
util::Status CreateWorld(const proto::ScenarioFiles& config,
                         std::unique_ptr<World>& world, Constants* constants) {

} // namespace setup
}  // namespace games

#endif
