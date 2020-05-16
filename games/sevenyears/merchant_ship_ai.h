#ifndef GAMES_SEVENYEARS_MERCHANT_SHIP_AI_H
#define GAMES_SEVENYEARS_MERCHANT_SHIP_AI_H

#include "games/actions/proto/plan.pb.h"
#include "games/actions/proto/strategy.pb.h"
#include "games/ai/unit_ai.h"
#include "games/sevenyears/interfaces.h"
#include "games/units/unit.h"
#include "util/status/status.h"

namespace sevenyears {

class SevenYears;

class SevenYearsMerchant : public ai::UnitAi {
public:
  SevenYearsMerchant(const sevenyears::SevenYearsState* seven) : game_(seven) {}
  util::Status AddStepsToPlan(const units::Unit& unit,
                              const actions::proto::Strategy& strategy,
                              actions::proto::Plan* plan) const override;
  util::Status Initialise();
  util::Status
  ValidMission(const actions::proto::SevenYearsMerchant& sym) const;

private:
  // Planning helper methods.
  util::Status planEuropeanTrade(const units::Unit& unit,
                             actions::proto::Plan* plan) const;
  util::Status planColonialTrade(const units::Unit& unit,
                             actions::proto::Plan* plan) const;
  util::Status planSupplyArmies(const units::Unit& unit,
                            actions::proto::Plan* plan) const;
  util::Status
  planReturnToBase(const units::Unit& unit,
                   const actions::proto::SevenYearsMerchant& strategy,
                   actions::proto::Plan* plan) const;

  const sevenyears::SevenYearsState* game_;
};

}  // namespace sevenyears

#endif
