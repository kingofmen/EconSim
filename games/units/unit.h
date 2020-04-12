#ifndef UNITS_UNIT_H
#define UNITS_UNIT_H

#include <string>
#include <unordered_map>

#include "games/geography/mobile.h"
#include "games/geography/proto/geography.pb.h"
#include "games/market/goods_utils.h"
#include "games/market/proto/goods.pb.h"
#include "games/units/proto/templates.pb.h"
#include "games/units/proto/units.pb.h"
#include "util/headers/int_types.h"
#include "util/proto/object_id.pb.h"

namespace units {

// Class to represent anything that moves about in the world, from mule trains
// to army groups.
class Unit : public geography::Mobile {
public:
  ~Unit();

  static std::unique_ptr<Unit> FromProto(const proto::Unit& proto);
  static bool RegisterTemplate(const proto::Template& proto);
  // TODO: Make this a StatusOr<Template&> when Abseil releases StatusOr.
  static const proto::Template* TemplateById(const util::proto::ObjectId& id);
  static const proto::Template* TemplateByKind(const std::string& kind);
  static Unit* ById(const util::proto::ObjectId& id);

  // Template and proto access.
  const proto::Unit& Proto() const { return proto_; }
  const util::proto::ObjectId& ID() const { return proto_.unit_id(); }
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

  // Returns the amount of good that can still be loaded, whether limited
  // by bulk or weight.
  market::Measure Capacity(const std::string& good) const;

  const std::string& template_kind() const { return proto_.unit_id().kind(); }

private:
  Unit(const proto::Unit& proto);

  static std::unordered_map<util::proto::ObjectId, Unit*> units_;

  proto::Unit proto_;
};

} // namespace units

#endif
