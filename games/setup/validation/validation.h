#ifndef GAME_SETUP_VALIDATION_VALIDATION_H
#define GAME_SETUP_VALIDATION_VALIDATION_H

#include <string>
#include <vector>

#include "games/setup/proto/setup.pb.h"

namespace games {
namespace setup {
namespace validation {

// Sanity-checks the protobufs, returning a list of errors.
std::vector<std::string> Validate(const games::setup::proto::Scenario& scenario,
                                  const games::setup::proto::GameWorld& world);

}  // namespace validation
}  // namespace setup
}  // namespace games



#endif
