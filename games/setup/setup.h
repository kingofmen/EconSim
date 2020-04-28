#ifndef GAMES_SETUP_SETUP_H
#define GAMES_SETUP_SETUP_H

#include <memory>
#include <vector>

#include "games/factions/factions.h"
#include "games/factions/proto/factions.pb.h"
#include "games/setup/proto/setup.pb.h"
#include "games/geography/connection.h"
#include "games/geography/geography.h"
#include "games/geography/proto/geography.pb.h"
#include "games/population/popunit.h"
#include "games/population/proto/population.pb.h"
#include "games/units/unit.h"
#include "src/google/protobuf/message.h"
#include "util/status/status.h"

namespace games {
namespace setup {

// Object to hold unchanging scenario information.
struct Constants {
  Constants() {}
  Constants(const Constants& other) = default;
  Constants(const games::setup::proto::Scenario& scenario);
  std::vector<population::proto::AutoProduction> auto_production_;
  std::vector<industry::proto::Production> production_chains_;
  // TODO: Don't store the subsistence levels twice.
  std::vector<population::proto::ConsumptionLevel> subsistence_;
  std::vector<population::proto::ConsumptionLevel> consumption_;
  market::proto::Container decay_rates_;
};

// Object to hold current state in memory.
struct World {
 public:
  // TODO: Make this a StatusOr when Abseil releases that.
  // Create a World from the proto object.
  static std::unique_ptr<World> FromProto(const proto::GameWorld& proto);

  // Save state to proto, which must not be null. Restores tags.
  util::Status ToProto(proto::GameWorld* proto);

  // World-state information.
  std::vector<std::unique_ptr<factions::FactionController>> factions_;
  std::vector<std::unique_ptr<population::PopUnit>> pops_;
  std::vector<std::unique_ptr<geography::Area>> areas_;
  std::vector<std::unique_ptr<geography::Connection>> connections_;
  std::vector<std::unique_ptr<units::Unit>> units_;

 private:
  // Restores ObjectId protos to have tags, where they exist.
  void restoreTags();
};

// Interface for passing world-state around.
class WorldStateInterface {
 public:
  virtual const World& World() const = 0;
  virtual const Constants& Constants() const = 0;
};

util::Status LoadScenario(const proto::ScenarioFiles& config,
                          proto::Scenario* scenario);

util::Status LoadWorld(const proto::ScenarioFiles& config,
                       proto::GameWorld* world);

// Load arbitrary protos with locations specified by the 'extras' field in the
// config.
util::Status
LoadExtras(const proto::ScenarioFiles& config,
           std::unordered_map<std::string, google::protobuf::Message*> extras);

// Canonicalises all ObjectIds in the provided proto, returning an
// error if it encounters any tags without referent.
util::Status CanonicaliseScenario(proto::Scenario* scenario);

// Canonicalises all ObjectIds in the provided proto, returning an
// error if it encounters any tags without referent.
util::Status CanonicaliseWorld(proto::GameWorld* world);

// Load world and constants protos from config. Note that this
// resets the world pointer.
util::Status CreateWorld(const proto::ScenarioFiles& config,
                         std::unique_ptr<World>& world, Constants* constants);

} // namespace setup
}  // namespace games

#endif
