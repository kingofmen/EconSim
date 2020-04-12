// Class to model land cargo carriers, from mules to semitrailers.
#ifndef BASE_UNIT_IMPL_LAND_CARGO_H
#define BASE_UNIT_IMPL_LAND_CARGO_H

#include "games/geography/proto/geography.pb.h"
#include "games/geography/mobile.h"
#include "games/market/proto/goods.pb.h"

namespace units {
namespace impl {

class LandCargoCarrier : public geography::Mobile {
public:
  uint64 speed_u(geography::proto::ConnectionType type) const override;

private:
  uint64 template_id_;
  market::proto::Container cargo_;
};

} // namespace impl
} // namespace units

#endif
