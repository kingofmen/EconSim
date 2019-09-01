#ifndef COLONY_FACTIONS_FACTIONS_H
#define COLONY_FACTIONS_FACTIONS_H

#include "colony/factions/proto/factions.pb.h"
#include "util/headers/int_types.h"

namespace factions {

// Information about the control and extent of a faction.
class FactionController {
 public:
   uint64 id() const { return proto_.id(); }

 private:
  // Wire format.
  proto::Faction proto_;
};

}  // namespace factions

#endif
