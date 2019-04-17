#ifndef GEOGRAPHY_MOBILE_HH
#define GEOGRAPHY_MOBILE_HH

#include "geography/proto/geography.pb.h"
#include "util/headers/int_types.h"

namespace geography {

// Interface that tracks the movement-related aspects of a unit - speed,
// location, transport capacity.
class Mobile {
public:
  // Returns the speed at which this unit travels in the terrain type.
  virtual uint64 speed_u(geography::proto::ConnectionType type) const = 0;
  virtual const geography::proto::Location& location() const = 0;
};

} // namespace geography

#endif
