#ifndef GAME_SETUP_VALIDATION_VALIDATION_H
#define GAME_SETUP_VALIDATION_VALIDATION_H

#include <string>
#include <vector>

#include "games/setup/proto/setup.pb.h"

namespace games {
namespace setup {
namespace validation {

typedef std::function<std::vector<std::string>(
    const games::setup::proto::GameWorld& world)>
    Validator;

// Registers an additional, game-specific validator.
void RegisterValidator(const std::string& key, Validator v);

// Sanity-checks the protobufs, returning a list of errors.
std::vector<std::string> Validate(const games::setup::proto::Scenario& scenario,
                                  const games::setup::proto::GameWorld& world);

// Optional validations which can apply to more than one game,
// but which won't be run unless registered using RegisterValidator.
namespace optional {

// Checks that world has factions and that every unit belongs to
// one that exists.
std::vector<std::string>
UnitFactions(const games::setup::proto::GameWorld& world);

}  // namespace optional
}  // namespace validation
}  // namespace setup
}  // namespace games



#endif
