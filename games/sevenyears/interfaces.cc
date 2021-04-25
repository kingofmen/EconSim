#include "games/sevenyears/interfaces.h"

#include "util/logging/logging.h"
#include "util/proto/object_id.pb.h"

namespace sevenyears {

const proto::AreaState& SevenYearsStateImpl::AreaState(
    const util::proto::ObjectId& area_id) const {
  if (area_states_.find(area_id) == area_states_.end()) {
    Log::Errorf("No state for area %s", util::objectid::DisplayString(area_id));
    static proto::AreaState dummy;
    *dummy.mutable_area_id() = util::objectid::kNullId;
    return dummy;
  }
  return area_states_.at(area_id);
}

sevenyears::proto::AreaState*
SevenYearsStateImpl::mutable_area_state(const util::proto::ObjectId& area_id) {
  if (area_states_.find(area_id) == area_states_.end()) {
    Log::Errorf("No state for area %s", util::objectid::DisplayString(area_id));
    static proto::AreaState dummy;
    return &dummy;
  }
  return &area_states_.at(area_id);
}

} // namespace sevenyears
