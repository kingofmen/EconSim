#ifndef GAMES_ACTIONS_STRATEGY_H
#define GAMES_ACTIONS_STRATEGY_H

#include <string>

#include "games/actions/proto/strategy.pb.h"
#include "games/actions/proto/plan.pb.h"
#include "util/status/status.h"

namespace actions {

// Registers strategy under its define field.
util::Status RegisterStrategy(const actions::proto::Strategy& strategy);

// Loads a previously-registered Strategy into the pointer, which must not be
// null.
util::Status LoadStrategy(const std::string& name,
                          actions::proto::Strategy* strategy);

const std::string& StepName(const actions::proto::Step& step);

}  // namespace actions

#endif
