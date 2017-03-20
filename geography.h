// Class to represent geographic locations.

#include "geography/proto/geography.pb.h"

namespace geography {

class Field : public proto::Field {
public:
  // Filters which return true if this Field has the requirements for at least
  // one variant in each step of the given production chain.
  bool HasFixedCapital(const industry::proto::Production &production) const;
  bool HasLandType(const industry::proto::Production &production) const;
  bool HasRawMaterials(const industry::proto::Production &production) const;
};

} // namespace geography
