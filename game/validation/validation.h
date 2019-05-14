#ifndef GAME_VALIDATION_VALIDATION_H
#define GAME_VALIDATION_VALIDATION_H

#include <string>
#include <vector>

#include "game/proto/game_world.pb.h"

namespace game {
namespace validation {

// Sanity-checks the protobufs, returning a list of errors.
std::vector<std::string> Validate(const game::proto::Scenario& scenario,
                                  const game::proto::GameWorld& world);

}  // namespace validation
}  // namespace game



#endif
