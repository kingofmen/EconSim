#include "games/units/unit.h"

#include <unordered_map>

#include "games/market/goods_utils.h"
#include "util/arithmetic/microunits.h"
#include "util/logging/logging.h"
#include "util/proto/object_id.h"
#include "util/proto/object_id.pb.h"

std::unordered_map<util::proto::ObjectId, const units::proto::Template> template_map_;
std::unordered_map<util::proto::ObjectId, units::Unit*> units::Unit::units_;

namespace units {

std::unique_ptr<Unit> Unit::FromProto(const proto::Unit& proto) {
  std::unique_ptr<Unit> ret;
  // TODO: Actually handle the errors when we get support for StatusOr<unique_ptr>.
  if (!proto.has_unit_id()) {
    Log::Warnf("Unit without ID: %s", proto.DebugString());
    return ret;
  }
  if (!proto.unit_id().has_kind()) {
    Log::Warnf("Unit without kind: %s", proto.DebugString());
    return ret;
  }
  if (TemplateByKind(proto.unit_id().kind()) == nullptr) {
    Log::Warnf("Unit with unknown template kind: %s", proto.DebugString());
    return ret;
  }
  if (ById(proto.unit_id()) != nullptr) {
    Log::Warnf("Duplicate unit: %s", proto.DebugString());
    return ret;
  }

  ret.reset(new Unit(proto));
  return ret;
}

bool Unit::RegisterTemplate(const proto::Template& proto) {
  if (!proto.has_template_id()) {
    return false;
  }

  if (template_map_.find(proto.template_id()) != template_map_.end()) {
    return false;
  }
  template_map_.emplace(proto.template_id(), proto);
  return true;
}

util::Status Unit::UnregisterTemplate(const util::proto::ObjectId& id) {
  if (template_map_.find(id) == template_map_.end()) {
    return util::NotFoundError("Template for unregistering not found.");
  }
  template_map_.erase(id);
  return util::OkStatus();
}

const proto::Template* Unit::TemplateById(const util::proto::ObjectId& id) {
  if (template_map_.find(id) == template_map_.end()) {
    return NULL;
  }
  return &template_map_.at(id);
}

const proto::Template* Unit::TemplateByKind(const std::string& kind) {
  util::proto::ObjectId id;
  id.set_kind(kind);
  return TemplateById(id);
}

Unit* Unit::ById(const util::proto::ObjectId& id) {
  return units_[id];
}

const proto::Template& Unit::Template() const {
  const proto::Template* t = TemplateByKind(proto_.unit_id().kind());
  if (!t) {
    Log::Errorf("Could not find template for unit ID %s",
                proto_.unit_id().DebugString());
  }
  return *t;
}

actions::proto::Strategy* Unit:: mutable_strategy() {
  return proto_.mutable_strategy();
}

actions::proto::Plan* Unit::mutable_plan() {
  return proto_.mutable_plan();
}

const actions::proto::Strategy& Unit:: strategy() const {
  return proto_.strategy();
}

const actions::proto::Plan& Unit::plan() const {
  return proto_.plan();
}

micro::uMeasure Unit::speed_u(geography::proto::ConnectionType type) const {
  return Template().mobility().speed_u();
}

const geography::proto::Location& Unit::location() const {
  return proto_.location();
}

geography::proto::Location* Unit::mutable_location() {
  return proto_.mutable_location();
}

const market::proto::Container& Unit::resources() const {
  return proto_.resources();
}

market::proto::Container* Unit::mutable_resources() {
  return proto_.mutable_resources();
}

Unit::Unit(const proto::Unit& proto) : proto_(proto), used_action_points_u(0) {
  units_[proto_.unit_id()] = this;
}

Unit::~Unit() { units_.erase(proto_.unit_id()); }

micro::Measure Unit::Capacity(const std::string& good) const {
  if (market::TransportType(good) == market::proto::TradeGood::TTT_IMMOBILE) {
    return 0;
  }
  micro::Measure current_bulk_u = 0;
  micro::Measure current_weight_u = 0;
  for (const auto& quantity : resources().quantities()) {
    current_bulk_u +=
        micro::MultiplyU(market::BulkU(quantity.first), quantity.second);
    current_weight_u +=
        micro::MultiplyU(market::WeightU(quantity.first), quantity.second);
  }

  // TODO: Scaling effects - also elsewhere!
  // TODO: Maybe this had better be cached?
  micro::Measure remaining_bulk_u = Template().mobility().max_bulk_u();
  micro::Measure remaining_weight_u = Template().mobility().max_weight_u();

  remaining_bulk_u -= current_bulk_u;
  remaining_weight_u -= current_weight_u;

  // Bulk and weight guaranteed nonzero.
  remaining_bulk_u = micro::DivideU(remaining_bulk_u, market::BulkU(good));
  remaining_weight_u =
      micro::DivideU(remaining_weight_u, market::WeightU(good));

  return std::min(remaining_bulk_u, remaining_weight_u);
}

micro::Measure Unit::action_points_u() const {
  micro::Measure base_u = Template().base_action_points_u();
  if (used_action_points_u > base_u) {
    return 0;
  }
  return base_u - used_action_points_u;
}

const util::proto::ObjectId& Unit::unit_id() const { return proto_.unit_id(); }
const util::proto::ObjectId& Unit::faction_id() const {
  return proto_.faction_id();
}


Unit* ById(const util::proto::ObjectId unit_id) {
  return Unit::ById(unit_id);
}



} // namespace units
