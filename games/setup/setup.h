#ifndef GAMES_SETUP_SETUP_H
#define GAMES_SETUP_SETUP_H

#include "games/setup/proto/setup.pb.h"
#include "util/status/status.h"

namespace games {
namespace setup {

util::Status LoadScenario(const proto::ScenarioFiles& config,
                          proto::Scenario* scenario);

util::Status LoadWorld(const proto::ScenarioFiles& config,
                       proto::GameWorld* world);

}  // namespace setup
}  // namespace games

#endif
