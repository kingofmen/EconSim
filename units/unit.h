#ifndef UNITS_UNIT_H
#define UNITS_UNIT_H

#include <unordered_map>

#include "geography/mobile.h"
#include "geography/proto/geography.pb.h"
#include "market/proto/goods.pb.h"
#include "units/proto/templates.pb.h"
#include "units/proto/units.pb.h"
#include "util/headers/int_types.h"
#include "util/proto/object_id.h"

namespace units {

// Class to represent anything that moves about in the world, from mule trains
// to army groups.
class Unit : public geography::Mobile {
public:
  ~Unit();

  static std::unique_ptr<Unit> FromProto(const proto::Unit& proto);
  static bool RegisterTemplate(const proto::Template& proto);
  // TODO: Make this a StatusOr<Template&> when Abseil releases StatusOr.
  static const proto::Template* TemplateById(uint64 id);
  static Unit* ById(const util::proto::ObjectId& id);

  // Template and proto access.
  const proto::Unit& Proto() const { return proto_; }
  const proto::Template& Template() const;

  // Planning interface.
  actions::proto::Strategy* mutable_strategy();
  actions::proto::Plan* mutable_plan();
  const actions::proto::Strategy& strategy();
  const actions::proto::Plan& plan();

  // From Mobile interface.
  uint64 speed_u(geography::proto::ConnectionType type) const override;
  const geography::proto::Location& location() const override;
  geography::proto::Location* mutable_location() override;

  // Cargo or supplies.
  const market::proto::Container& resources() const;
  market::proto::Container* mutable_resources();

private:
  Unit(const proto::Unit& proto);

  static std::unordered_map<uint64, const proto::Template> templates_;
  static std::unordered_map<util::proto::ObjectId, Unit*> units_;

  proto::Unit proto_;
};

} // namespace units

#endif
