#ifndef GAMES_SEVENYEARS_ARMY_AI_H
#define GAMES_SEVENYEARS_ARMY_AI_H

#include "games/actions/proto/plan.pb.h"
#include "games/actions/proto/strategy.pb.h"
#include "games/ai/unit_ai.h"
#include "games/sevenyears/interfaces.h"

namespace sevenyears {

class SevenYearsArmyAi : public ai::UnitAi {
public:
  SevenYearsArmyAi(const sevenyears::SevenYearsState* seven) : game_(seven) {}

  util::Status AddStepsToPlan(const units::Unit& unit,
                              const actions::proto::Strategy& strategy,
                              actions::proto::Plan* plan) override;
  util::Status Initialise();

private:
  const sevenyears::SevenYearsState* game_;
};

} // namespace sevenyears

#endif
