#include "games/factions/factions.h"

#include "util/arithmetic/bits.h"
#include "util/proto/object_id.h"

namespace factions {

std::unordered_map<uint64, FactionController*> FactionController::faction_map_;
std::unordered_map<util::proto::ObjectId, FactionController*> id_faction_map_;

FactionController::FactionController(const proto::Faction& p)
    : proto_(p), privileges_(p.privileges().begin(), p.privileges().end()) {
  if (proto_.has_faction_id()) {
    id_faction_map_[faction_id()] = this;
    faction_map_[faction_id().number()] = this;
  }
  if (proto_.has_id()) {
    faction_map_[id()] = this;
  }

  for (const uint64 pop_id : proto_.pop_ids()) {
    citizens_.insert(pop_id);
  }
}

const util::proto::ObjectId& FactionController::faction_id() const {
  return proto_.faction_id();
}

bool FactionController::IsFullCitizen(uint64 pop_id) const {
  return citizens_.find(pop_id) != citizens_.end();
}

FactionController* FactionController::GetByID(uint64 id) {
  return faction_map_[id];
}

FactionController* FactionController::GetByID(const util::proto::ObjectId& id) {
  if (id_faction_map_.find(id) == id_faction_map_.end()) {
    return NULL;
  }
  return id_faction_map_.at(id);
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
  if (!proto.has_id() && !proto.has_faction_id()) {
    return ret;
  }
  ret.reset(new FactionController(proto));
  return ret;
}

std::vector<std::pair<std::vector<util::proto::ObjectId>,
                      std::vector<util::proto::ObjectId>>>
Divide(const std::vector<util::proto::ObjectId> factions,
       util::objectid::Predicate willAlly,
       util::objectid::Predicate willFight) {
  std::vector<std::pair<std::vector<util::proto::ObjectId>,
                        std::vector<util::proto::ObjectId>>>
      conflicts;

  auto numFactions = factions.size();
  std::vector<bits::Mask> alliance;
  std::vector<bits::Mask> hostile;
  for (unsigned int ii = 0; ii < numFactions; ++ii) {
    alliance.push_back(bits::GetMask(ii));
    hostile.push_back(bits::kEmpty);
  }
  for (unsigned int ii = 0; ii < numFactions; ++ii) {
    for (unsigned int jj = ii+1; jj < numFactions; ++jj) {
      if (ii == jj) {
        continue;
      }
      if (willAlly(factions[ii], factions[jj])) {
        alliance[ii].set(jj);
        alliance[jj].set(ii);
      }
      if (willFight(factions[ii], factions[jj])) {
        hostile[ii].set(jj);
        hostile[jj].set(ii);
      }
    }
  }

  std::vector<bits::Mask> combats;
  for (unsigned int ii = 0; ii < numFactions; ++ii) {
    for (unsigned int jj = ii+1; jj < numFactions; ++jj) {
      if (!hostile[ii].test(jj)) {
        continue;
      }

      auto combat = bits::GetMask(ii, jj);
      auto conflict = std::pair<std::vector<util::proto::ObjectId>,
                                std::vector<util::proto::ObjectId>>(
          {factions[ii]}, {factions[jj]});
      auto oneSide = bits::GetMask(ii);
      auto otherSide = bits::GetMask(jj);
      for (unsigned int kk = 0; kk < numFactions; ++kk) {
        if (kk == ii || kk == jj) {
          continue;
        }
        auto support = alliance[kk] & oneSide;
        if (support == oneSide) {
          auto oppose = hostile[kk] & otherSide;
          if (oppose == otherSide) {
            oneSide.set(kk);
            conflict.first.push_back(factions[kk]);
            continue;
          }
        }
        support = alliance[kk] & otherSide;
        if (support == otherSide) {
          auto oppose = hostile[kk] & oneSide;
          if (oppose == oneSide) {
            conflict.second.push_back(factions[kk]);
            otherSide.set(kk);
          }
        }
      }
      bool seen = false;
      auto current = oneSide | otherSide;
      for (const auto& combat : combats) {
        if (bits::Subset(current, combat)) {
          seen = true;
          break;
        }
      }
      if (seen) {
        continue;
      }
      conflicts.push_back(conflict);
      combats.push_back(current);
    }
  }
  return conflicts;
}

} // namespace factions
