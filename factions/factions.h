#ifndef FACTIONS_FACTIONS_H
#define FACTIONS_FACTIONS_H

#include <unordered_set>

#include "factions/proto/factions.pb.h"
#include "util/headers/int_types.h"

namespace factions {

// Information about the control and extent of a faction.
class FactionController {
 public:
  // Constructor.
  FactionController(const proto::Faction& p);

  // Returns the faction id.
  uint64 id() const { return proto_.id(); }

  // Returns true if the POP is a direct member of the faction, a full citizen.
  bool IsFullCitizen(uint64 pop_id) const;

  // Read access to the protobuf.
  const proto::Faction& proto() { return proto_; }

private:
  // Wire format.
  proto::Faction proto_;
  // Citizen IDs.
  std::unordered_set<uint64> citizens_;
};

}  // namespace factions

#endif
