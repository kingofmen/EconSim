#include "factions/factions.h"

namespace factions {

std::unordered_map<uint64, FactionController*> FactionController::faction_map_;

FactionController::FactionController(const proto::Faction& p)
    : proto_(p), privileges_(p.privileges().begin(), p.privileges().end()) {
  for (const uint64 pop_id : proto_.pop_ids()) {
    citizens_.insert(pop_id);
  }
  faction_map_[id()] = this;
}

bool FactionController::IsFullCitizen(uint64 pop_id) const {
  return citizens_.find(pop_id) != citizens_.end();
}

FactionController* FactionController::GetByID(uint64 id) {
  return faction_map_[id];
}

bool FactionController::HasPrivileges(uint64 pop_id, int32 mask) const {
  const auto& it = privileges_.find(pop_id);
  if (it == privileges_.end()) {
    return false;
  }
  return (it->second & mask) == mask;
}

bool FactionController::HasAnyPrivilege(uint64 pop_id, int32 mask) const {
  const auto& it = privileges_.find(pop_id);
  if (it == privileges_.end()) {
    return false;
  }
  return it->second & mask;
}

std::unique_ptr<FactionController>
FactionController::FromProto(const proto::Faction& proto) {
  std::unique_ptr<FactionController> ret;
  // TODO: Actually handle the errors when we get support for
  // StatusOr<unique_ptr>.
  if (!proto.has_id()) {
    return ret;
  }
  ret.reset(new FactionController(proto));
  return ret;
}

} // namespace factions
