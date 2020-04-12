#include "games/units/impl/land_cargo.h"

namespace units {
namespace impl {

uint64 LandCargoCarrier::speed_u(geography::proto::ConnectionType type) const {
  return 1;
}

} // namespace impl
} // namespace units
