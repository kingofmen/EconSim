#ifndef GAMES_ACTIONS_STRATEGY_H
#define GAMES_ACTIONS_STRATEGY_H

#include <string>

#include "games/actions/proto/strategy.pb.h"
#include "util/status/status.h"

namespace actions {

// Registers strategy under its define field.
util::Status RegisterStrategy(const actions::proto::Strategy& strategy);

// Loads a previous-registered Strategy into the pointer, which must not be
// null.
util::Status LoadStrategy(const std::string& name,
                          actions::proto::Strategy* strategy);

}  // namespace actions

#endif
