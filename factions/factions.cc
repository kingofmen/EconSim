#include "factions/factions.h"

namespace factions {

FactionController::FactionController(const proto::Faction& p) : proto_(p) {
  for (const uint64 pop_id : proto_.pop_ids()) {
    citizens_.insert(pop_id);
  }
}

bool FactionController::IsFullCitizen(uint64 pop_id) const {
  return citizens_.find(pop_id) != citizens_.end();
}

}  // namespace factions
