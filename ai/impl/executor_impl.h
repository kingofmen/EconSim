#ifndef AI_IMPL_EXECUTOR_IMPL_H
#define AI_IMPL_EXECUTOR_IMPL_H

#include "actions/proto/plan.pb.h"
#include "units/unit.h"

namespace ai {
namespace impl {

// Move unit along the connection in step. Returns true if the unit moves.
bool MoveUnit(const actions::proto::Step& step, units::Unit* unit);

// Registers a buy or sell order with the local market; returns false if there
// isn't one, or the order otherwise fails.
bool BuyOrSell(const actions::proto::Step& step, units::Unit* unit);


} // namespace impl
} // namespace ai

#endif
