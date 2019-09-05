#ifndef FACTIONS_FACTIONS_H
#define FACTIONS_FACTIONS_H

#include <unordered_map>
#include <unordered_set>

#include "factions/proto/factions.pb.h"
#include "util/headers/int_types.h"

namespace factions {

// Information about the control and extent of a faction.
class FactionController {
 public:
   static std::unique_ptr<FactionController>
   FromProto(const proto::Faction& proto);

   // Returns the faction id.
   uint64 id() const { return proto_.id(); }

   // Returns true if the POP is a direct member of the faction, a full citizen.
   bool IsFullCitizen(uint64 pop_id) const;

   // Returns the controller with the given ID.
   static FactionController* GetByID(uint64 id);

   // Read access to the protobuf.
   const proto::Faction& Proto() { return proto_; }

 private:
   // Constructor.
   FactionController(const proto::Faction& p);

   // Wire format.
   proto::Faction proto_;
   // Citizen IDs.
   std::unordered_set<uint64> citizens_;
   // Lookup map.
   static std::unordered_map<uint64, FactionController*> faction_map_;
};

}  // namespace factions

#endif
