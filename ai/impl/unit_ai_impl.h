#ifndef AI_IMPL_UNIT_AI_IMPL_H
#define AI_IMPL_UNIT_AI_IMPL_H

#include "actions/proto/strategy.pb.h"
#include "actions/proto/plan.pb.h"
#include "ai/unit_ai.h"
#include "units/unit.h"

namespace ai {
namespace impl {

class ShuttleTrader : public ai::UnitAi {
public:
  void AddStepsToPlan(const units::Unit& unit,
                      actions::proto::Strategy* strategy,
                      actions::proto::Plan* plan) const override;

private:
};

} // namespace impl
} // namespace ai

#endif
