#include "games/sevenyears/army_ai.h"

#include "games/ai/planner.h"

namespace sevenyears {

util::Status
SevenYearsArmyAi::AddStepsToPlan(const units::Unit& unit,
                                 const actions::proto::Strategy& strategy,
                                 actions::proto::Plan* plan) {
  if (!strategy.has_seven_years_army()) {
    return util::NotFoundError("No SevenYearsArmy strategy");
  }
  if (plan->steps_size() > 0) {
    return util::OkStatus();
  }

  return util::OkStatus();
}

util::Status SevenYearsArmyAi::Initialise() {
  actions::proto::Strategy strategy;
  strategy.mutable_seven_years_army()->mutable_base_area_id()->set_number(
      1);
  return ai::RegisterPlanner(strategy, this);
}

} // namespace sevenyears
