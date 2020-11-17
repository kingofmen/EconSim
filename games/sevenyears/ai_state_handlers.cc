#include "games/sevenyears/ai_state_handlers.h"

#include "games/actions/proto/plan.pb.h"
#include "games/ai/executer.h"
#include "games/ai/impl/ai_utils.h"
#include "games/market/goods_utils.h"
#include "games/sevenyears/constants.h"
#include "games/sevenyears/interfaces.h"
#include "games/units/unit.h"
#include "util/arithmetic/microunits.h"
#include "util/logging/logging.h"
#include "util/proto/object_id.pb.h"

namespace sevenyears {
namespace {

} // namespace

sevenyears::proto::LocalFactionInfo*
FindLocalFactionInfo(const util::proto::ObjectId& faction_id,
                     proto::AreaState* state) {
  for (int i = 0; i < state->factions_size(); ++i) {
    auto* lfi = state->mutable_factions(i);
    if (lfi->faction_id() == faction_id) {
      return lfi;
    }
  }
  Log::Debugf("Creating faction info for %s in %s",
              util::objectid::DisplayString(faction_id),
              util::objectid::DisplayString(state->area_id()));
  auto* lfi = state->add_factions();
  *lfi->mutable_faction_id() = faction_id;
  return lfi;
}

void CreateExpectedArrivals(const units::Unit& unit,
                            const actions::proto::Plan& plan,
                            SevenYearsState* world_state) {
  const util::proto::ObjectId& faction_id = unit.faction_id();
  uint64 expected_time = world_state->timestamp();
  util::proto::ObjectId current_area_id = unit.location().a_area_id();
  micro::uMeasure current_action_points_u = unit.action_points_u();
  market::proto::Container projected_cargo;
  market::Copy(unit.resources(), unit.resources(), &projected_cargo);
  for (int idx = 0; idx < plan.steps_size(); ++idx) {
    // Special case for movement.
    std::vector<uint64> path_ids;
    while (idx < plan.steps_size() &&
           plan.steps(idx).action() == actions::proto::AA_MOVE) {
      auto connection_id = plan.steps(idx).connection_id();
      path_ids.push_back(connection_id);
      auto* conn = geography::Connection::ById(connection_id);
      if (conn == nullptr) {
        continue;
      }
      current_area_id = conn->OtherSide(current_area_id);
      idx++;
    }
    if (idx >= plan.steps_size()) {
      // Reached end of Plan with moves, so exit.
      break;
    }
    if (!path_ids.empty()) {
      expected_time += ai::utils::NumTurns(unit, path_ids);
    }

    // Handle other actions.
    const auto& step = plan.steps(idx);
    auto cost_u = ai::GetCost(step, unit);
    current_action_points_u -= cost_u;
    if (current_action_points_u < 0) {
      expected_time++;
      current_action_points_u = unit.action_points_u();
    }
    switch (step.trigger_case()) {
      case actions::proto::Step::kAction:
        // Movement already handled, nothing else to do.
      break;
      case actions::proto::Step::kKey:
        if (step.key() == constants::OffloadCargo()) {
          if (market::Empty(projected_cargo)) {
            break;
          }
          auto* state = world_state->mutable_area_state(current_area_id);
          auto* lfi = FindLocalFactionInfo(faction_id, state);
          auto* arrival = lfi->add_arrivals();
          arrival->set_timestamp(expected_time);
          *arrival->mutable_cargo() << projected_cargo;
          *arrival->mutable_unit_id() = unit.unit_id();
        } else if (step.key() == constants::LoadShip()) {
          auto good = step.good();
          if (good.empty()) {
            Log::Warn("Empty load-cargo step, ignoring");
            break;
          }
          // TODO: Improve this calculation?
          market::SetAmount(good, unit.TotalCapacity(good), &projected_cargo);
        }
        break;
      default: break;
    }
  }
}

void RegisterArrival(const units::Unit& unit,
                     const util::proto::ObjectId& area_id,
                     SevenYearsState* world_state) {
  auto* state = world_state->mutable_area_state(area_id);
  const util::proto::ObjectId& faction_id = unit.faction_id();
  auto* lfi = FindLocalFactionInfo(faction_id, state);
  const util::proto::ObjectId& unit_id = unit.unit_id();
  auto* arrivals = lfi->mutable_arrivals();

  for (auto& curr = arrivals->cbegin(); curr != arrivals->cend(); ++curr) {
    if (curr->unit_id() != unit_id) {
      continue;
    }
    curr = arrivals->erase(curr);
    // Remove only the first expected arrival.
    break;
  }
}

} // namespace sevenyears

