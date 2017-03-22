// Class to represent geographic locations.

#include "geography/proto/geography.pb.h"

namespace geography {

  // Filters which return true if this Field has the requirements for at least
  // one variant in each step of the given production chain.
bool HasFixedCapital(const proto::Field &field,
                     const industry::proto::Production &production);
bool HasLandType(const proto::Field &field,
                 const industry::proto::Production &production);
bool HasRawMaterials(const proto::Field &field,
                     const industry::proto::Production &production);

class Area : public proto::Area {
 public:
  Area() = default;
  void Update();
};

} // namespace geography
