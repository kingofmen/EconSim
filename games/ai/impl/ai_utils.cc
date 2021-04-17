#include "games/ai/impl/ai_utils.h"

#include "games/geography/connection.h"
#include "games/units/unit.h"
#include "util/arithmetic/microunits.h"
#include "util/logging/logging.h"

namespace ai {
namespace utils {

micro::Measure GetProgress(const micro::Measure fraction_u, const units::Unit& unit,
                           const geography::Connection& conn) {
  uint64 distance_u = unit.speed_u(conn.type());
  distance_u = micro::MultiplyU(distance_u, fraction_u);
  return distance_u;
}

int NumTurns(const units::Unit& unit, const std::vector<uint64> path) {
  int turns = 0;
  for (const auto pid : path) {
    const geography::Connection* conn = geography::Connection::ById(pid);
    if (conn == nullptr) {
      // Traversing nonexistent connections presumably takes zero time.
      DLOGF(Log::P_DEBUG, "Attempted to traverse nonexistent connection %d",
            pid);
      continue;
    }
    micro::Measure progress_u = 0;
    while (progress_u < conn->length_u()) {
      turns++;
      // TODO: This assumes full progress every time.
      auto distance_u = GetProgress(micro::kOneInU, unit, *conn);
      if (distance_u < 1) {
        // Exit out of this special case.
        break;
      }
      progress_u += distance_u;
    }
  }
  return turns;
}


} // namespace utils
} // namespace ai
