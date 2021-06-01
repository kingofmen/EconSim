#include "games/sevenyears/battles.h"

#include "games/units/unit.h"
#include "util/logging/logging.h"

namespace sevenyears {
namespace {

// Returns the distance at which one movement catches up to the other.
micro::Measure
sameDirectionInterception(const geography::Connection::Movement& movement,
                          const geography::Connection::Movement& otherMove) {
  if (movement.start_u == otherMove.start_u) {
    return movement.start_u;
  }
  if (movement.distance_u == otherMove.distance_u) {
    // Speeds are equal so they will never intercept unless they start
    // out at the same point, which we already checked for.
    return -1;
  }

  // It does not matter which way the movements go. Suppose the movements
  // are from points A to Z and a to z, and the interception point is X.
  // If you run the movements in reverse you will see them going at the
  // same speeds from (Z, z) to (A, a), and they will still intercept at
  // point X. So we can assume they are starting at points (A, a).
  // Let S and s be the speeds, then the interception time x satisfies:
  //   A + xS = a + xs.
  auto time_u = micro::DivideU(movement.start_u - otherMove.start_u,
                               otherMove.distance_u - movement.distance_u);
  return micro::MultiplyU(time_u, movement.distance_u);
}

} // namespace

micro::Measure Interception(const geography::Connection::Movement& movement,
                            const geography::Connection::Movement& otherMove,
                            micro::Measure length_u) {
  if (movement.base_area_id == otherMove.base_area_id) {
    return sameDirectionInterception(movement, otherMove);
  }

  // Interception time is distance divided by combined speed, which may be
  // negative.
  auto distance_u = length_u - movement.start_u - otherMove.start_u;
  auto time_u =
      micro::DivideU(distance_u, (movement.distance_u + otherMove.distance_u));
  return movement.start_u + micro::MultiplyU(time_u, movement.distance_u);
}


void SeaMoveObserver::Listen(const geography::Connection::Movement& movement) {}

void SeaMoveObserver::Battle(BattleResolver& resolver) {}

void SeaMoveObserver::Clear() {}

void LandMoveObserver::Listen(const geography::Connection::Movement& movement) {
  const auto& conn_id = movement.connection_id;
  traversals_[conn_id].push_back(movement);
}

void LandMoveObserver::Battle(BattleResolver& resolver) {
  for (const auto& it : traversals_) {
    const auto& conn_id = it.first;
    const auto* connection = geography::Connection::ById(conn_id);
    const auto& a_area_id = connection->a_id();
    const auto& z_area_id = connection->z_id();
    const auto length_u = connection->length_u();

    std::vector<Encounter> meetings;
    
    std::unordered_set<util::proto::ObjectId> faction_ids;
    auto& movements = it.second;
    for (unsigned int ii = 0; ii < movements.size(); ++ii) {
      auto& movement = movements[ii];
      auto* unit = units::Unit::ById(movement.object_id);
      if (unit == nullptr) {
        // This should never happen.
        continue;
      }

      const auto& faction_id = unit->faction_id();
      // Check if this unit encounters any existing meeting.
      bool forwards = movement.base_area_id == a_area_id;
      // pointA is the closest the unit gets to area A, whether
      // at the beginning or end of its move.
      micro::uMeasure pointA =
          forwards ? movement.start_u
                   : length_u - movement.start_u - movement.distance_u;
      micro::uMeasure pointZ = pointA + movement.distance_u;
      Encounter* bestMeeting = nullptr;
      for (auto& cand : meetings) {
        if (cand.point_u < pointA) {
          continue;
        }
        if (cand.point_u > pointZ) {
          continue;
        }
        if (bestMeeting == nullptr) {
          bestMeeting = &cand;
          continue;
        }
        if (forwards) {
          if (cand.point_u > bestMeeting->point_u) {
            continue;
          }
        } else if (cand.point_u < bestMeeting->point_u) {
          continue;
        }
        bestMeeting = &cand;
      }

      // This is an approximation since it's possible
      // that one of the not-yet-considered encounters
      // below would be better (and indeed would abrogate
      // some or all of these already-existing meetings).
      // But it's good enough.
      if (bestMeeting != nullptr) {
        bestMeeting->armies[faction_id].push_back(unit);
        continue;
      }

      // No existing meeting. See if any are created.
      Encounter tempMeeting;
      tempMeeting.point_u = micro::kMaxU;
      for (unsigned int jj = ii+1; jj < movements.size(); ++jj) {
        const auto& otherMove = movements[jj];
        auto* cand = units::Unit::ById(otherMove.object_id);
        if (cand == nullptr) {
          // This should never happen.
          continue;
        }
        // TODO: Implement an actual hostility-checker here.
        if (util::objectid::Equal(faction_id, cand->faction_id())) {
          continue;
        }
        const auto intercept_u = Interception(movement, otherMove, length_u);
        if (intercept_u < movement.start_u) {
          continue;
        }
        if (intercept_u > movement.start_u + movement.distance_u) {
          continue;
        }
        if (intercept_u < tempMeeting.point_u) {
          tempMeeting.point_u = intercept_u;
          tempMeeting.armies.clear();
          tempMeeting.armies[faction_id].push_back(unit);
          tempMeeting.armies[cand->faction_id()].push_back(cand);
        }
      }
      if (!tempMeeting.armies.empty()) {
        meetings.push_back(tempMeeting);
      }
    }
    for (auto& meeting : meetings) {
      resolver.Resolve(meeting);
    }
  }
}

void LandMoveObserver::Clear() {
  traversals_.clear();
}

} // namespace sevenyears
