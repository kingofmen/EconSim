// Class to represent geographic locations.

#include "geography/proto/geography.pb.h"

namespace geography {

class Field : public proto::Field {
public:
  void PossibleProductionChains(
      const std::vector<const industry::proto::Production *> &candidates,
      std::vector<const industry::proto::Production *> *possible) const;
};

} // namespace geography
