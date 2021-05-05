#ifndef GAMES_SEVENYEARS_MERCHANT_SHIP_AI_H
#define GAMES_SEVENYEARS_MERCHANT_SHIP_AI_H

#include "games/actions/proto/plan.pb.h"
#include "games/actions/proto/strategy.pb.h"
#include "games/ai/unit_ai.h"
#include "games/sevenyears/interfaces.h"
#include "games/sevenyears/proto/ai_state.pb.h"
#include "games/units/unit.h"
#include "util/proto/object_id.h"
#include "util/proto/object_id.pb.h"
#include "util/status/status.h"

namespace sevenyears {

class SevenYears;

// Struct to keep track of candidate for trade paths. Exposed for testing.
// Missions operate with at most four ports, not all of which need be
// distinct: Current location, (optional) pickup, target, and dropoff.
struct PlannedPath {
  // Expected supplies carried home or delivered.
  micro::Measure supplies;
  // Expected trade goods carried to a supply source.
  micro::Measure trade_goods;
  // Time in turns from current location to either pickup or target.
  int first_traverse_time;
  // Time from pickup to target.
  int pickup_to_target_time;
  // Distance from target to dropoff.
  int target_to_dropoff_time;
  // The target port.
  util::proto::ObjectId target_port_id;
  // The home port supplying the cargo.
  util::proto::ObjectId pickup_id;
  // The home port receiving supplies.
  util::proto::ObjectId dropoff_id;
  // Importance of the trade; supplies delivered and created divided by time
  // taken.
  micro::Measure goodness;
};

class SevenYearsMerchant : public ai::UnitAi {
public:
  SevenYearsMerchant(const sevenyears::SevenYearsState* seven) : game_(seven) {}
  util::Status AddStepsToPlan(const units::Unit& unit,
                              const actions::proto::Strategy& strategy,
                              actions::proto::Plan* plan) override;
  util::Status Initialise();
  util::Status
  ValidMission(const actions::proto::SevenYearsMerchant& sym) const;

private:
  typedef std::function<util::Status(const geography::Area& area,
                                     const sevenyears::proto::AreaState& state,
                                     const util::proto::ObjectId& faction_id)>
      MissionCheck;
  typedef std::function<micro::Measure(micro::Measure, micro::Measure,
                                       micro::Measure)>
      GoodnessMetric;

  // Planning helper methods.
  void checkForDropoff(const units::Unit& unit,
                       const util::proto::ObjectId& area_id,
                       const util::proto::ObjectId& faction_id,
                       GoodnessMetric metric, PlannedPath* candidate);
  void checkForPickup(const units::Unit& unit,
                      const util::proto::ObjectId& area_id,
                      const util::proto::ObjectId& faction_id,
                      GoodnessMetric metric, PlannedPath* candidate);
  util::Status createCandidatePath(const units::Unit& unit,
                                   const geography::Area& area,
                                   const util::proto::ObjectId& faction_id,
                                   MissionCheck pred, PlannedPath* candidate);
  util::Status
  planEuropeanTrade(const units::Unit& unit,
                    const actions::proto::SevenYearsMerchant& strategy,
                    actions::proto::Plan* plan);
  util::Status planColonialTrade(const units::Unit& unit,
                                 actions::proto::Plan* plan) const;
  util::Status planSupplyArmies(const units::Unit& unit,
                                actions::proto::Plan* plan);
  util::Status
  planReturnToBase(const units::Unit& unit,
                   const actions::proto::SevenYearsMerchant& strategy,
                   actions::proto::Plan* plan) const;

  std::unordered_map<util::proto::ObjectId, sevenyears::proto::Faction>
      factions_;
  const sevenyears::SevenYearsState* game_;
};

}  // namespace sevenyears

#endif
