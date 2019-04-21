#ifndef AI_IMPL_EXECUTOR_IMPL_H
#define AI_IMPL_EXECUTOR_IMPL_H

#include "actions/proto/plan.pb.h"
#include "units/unit.h"

namespace ai {
namespace impl {

bool MoveUnit(const actions::proto::Step& step, units::Unit* unit);

} // namespace impl
} // namespace ai

#endif
