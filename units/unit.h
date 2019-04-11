#ifndef UNITS_UNIT_H
#define UNITS_UNIT_H

#include <unordered_map>

#include "geography/mobile.h"
#include "geography/proto/geography.pb.h"
#include "units/proto/templates.pb.h"
#include "units/proto/units.pb.h"
#include "util/headers/int_types.h"

namespace units {

// Class to represent anything that moves about in the world, from mule trains
// to army groups.
class Unit : public geography::Mobile {
public:
  ~Unit();

  static std::unique_ptr<Unit> FromProto(const proto::Unit& proto);
  static bool RegisterTemplate(const proto::Template& proto);
  static const proto::Template* TemplateById(uint64 id);

  // From Mobile interface.
  uint64 speed_u(geography::proto::ConnectionType type) const override;

private:
  Unit(const proto::Unit& proto);

  static std::unordered_map<uint64, const proto::Template> templates_;

  proto::Unit proto_;
};

} // namespace units

#endif
