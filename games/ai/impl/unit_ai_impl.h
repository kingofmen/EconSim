#ifndef GAMES_AI_IMPL_UNIT_AI_IMPL_H
#define GAMES_AI_IMPL_UNIT_AI_IMPL_H

#include "actions/proto/strategy.pb.h"
#include "actions/proto/plan.pb.h"
#include "games/ai/unit_ai.h"
#include "units/unit.h"
#include "util/status/status.h"

namespace ai {
namespace impl {

class ShuttleTrader : public ai::UnitAi {
public:
  util::Status AddStepsToPlan(const units::Unit& unit,
                              const actions::proto::Strategy& strategy,
                              actions::proto::Plan* plan) const override;

private:
};

class SevenYearsMerchant : public ai::UnitAi {
public:
  util::Status AddStepsToPlan(const units::Unit& unit,
                              const actions::proto::Strategy& strategy,
                              actions::proto::Plan* plan) const override;

private:
};

} // namespace impl
} // namespace ai

#endif
