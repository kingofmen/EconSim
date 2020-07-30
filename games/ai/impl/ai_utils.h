#ifndef GAMES_AI_IMPL_AI_UTILS_H
#define GAMES_AI_IMPL_AI_UTILS_H

#include <vector>

#include "games/geography/connection.h"
#include "games/units/unit.h"
#include "util/arithmetic/microunits.h"

namespace ai {
namespace utils {

// Returns the distance the unit traverses along the connection.
micro::Measure GetProgress(const micro::Measure cost_u, const units::Unit& unit,
                           const geography::Connection& conn);

// Returns the number of turns the unit will take to traverse the path.
int NumTurns(const units::Unit& unit, const std::vector<uint64> path);

} // namespace utils
} // namespace ai

#endif
