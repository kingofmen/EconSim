// Package for methods that change world state, as opposed to state internal
// to the SevenYears game object.
#ifndef GAMES_SEVENYEARS_AI_STATE_HANDLERS_H
#define GAMES_SEVENYEARS_AI_STATE_HANDLERS_H

#include "games/actions/proto/plan.pb.h"
#include "games/sevenyears/interfaces.h"
#include "games/units/unit.h"
#include "util/proto/object_id.pb.h"

namespace sevenyears {

// Creates ExpectedArrival objects in ports the unit plans to arrive at.
void CreateExpectedArrivals(const units::Unit& unit,
                            const actions::proto::Plan& plan,
                            SevenYearsState* world_state);

// Returns the area-local faction information, creating it if necessary.
sevenyears::proto::LocalFactionInfo*
FindLocalFactionInfo(const util::proto::ObjectId& faction_id,
                     proto::AreaState* state);

// Remove first expected-arrival object associated with the unit in the
// relevant area.
void RegisterArrival(const units::Unit& unit,
                     const util::proto::ObjectId& area_id,
                     const market::proto::Container& loaded,
                     SevenYearsState* world_state);

} // namespace sevenyears

#endif
