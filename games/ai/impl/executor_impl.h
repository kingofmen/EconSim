#ifndef GAMES_AI_IMPL_EXECUTOR_IMPL_H
#define GAMES_AI_IMPL_EXECUTOR_IMPL_H

#include "games/actions/proto/plan.pb.h"
#include "games/units/unit.h"
#include "util/status/status.h"

namespace ai {
namespace impl {

// Move unit along the connection in step. Returns OK if the unit moves.
util::Status MoveUnit(const actions::proto::Step& step, units::Unit* unit);

// Registers a buy or sell order with the local market; returns an error if
// there isn't one, or the order otherwise fails.
util::Status BuyOrSell(const actions::proto::Step& step, units::Unit* unit);

// Flips the unit's Strategy into its next state.
util::Status SwitchState(const actions::proto::Step& step, units::Unit* unit);

// Turns the unit around in its current connection.
util::Status TurnAround(const actions::proto::Step& step, units::Unit* unit);

} // namespace impl
} // namespace ai

#endif
