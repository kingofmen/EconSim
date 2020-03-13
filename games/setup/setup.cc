#include "games/setup/setup.h"

namespace games {
namespace setup {

util::Status LoadScenario(const proto::ScenarioFiles& config,
                          proto::Scenario* scenario) {
  return util::OkStatus();
}

util::Status LoadWorld(const proto::ScenarioFiles& config,
                       proto::GameWorld* world) {
  return util::OkStatus();
}

}  // namespace setup
}  // namespace games
