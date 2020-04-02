#include "units/unit.h"

#include <unordered_map>

#include "util/arithmetic/microunits.h"
#include "util/logging/logging.h"
#include "util/proto/object_id.h"
#include "util/proto/object_id.pb.h"

std::unordered_map<uint64, const units::proto::Template> units::Unit::templates_;
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
  if (!proto.unit_id().has_type() && !proto.unit_id().has_kind()) {
    Log::Warnf("Unit without type or kind: %s", proto.DebugString());
    return ret;
  }
  if (proto.unit_id().has_kind()) {
    if (TemplateByKind(proto.unit_id().kind()) == nullptr) {
      Log::Warnf("Unit with unknown template kind: %s", proto.DebugString());
      return ret;
    }
  } else if (TemplateById(proto.unit_id().type()) == nullptr) {
    Log::Warnf("Unit with unknown template type: %s", proto.DebugString());
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
  if (!proto.has_id() && !proto.has_template_id()) {
    return false;
  }

  if (proto.has_id()) {
    uint64 id = proto.id();
    if (TemplateById(id) != nullptr) {
      return false;
    }
    templates_.insert({id, proto});
    return true;
  }

  if (template_map_.find(proto.template_id()) != template_map_.end()) {
    return false;
  }
  template_map_.emplace(proto.template_id(), proto);
  return true;
}

const proto::Template* Unit::TemplateById(uint64 id) {
  if (templates_.find(id) == templates_.end()) {
    return NULL;
  }
  return &templates_[id];
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
  const proto::Template* t = TemplateById(proto_.unit_id().type());
  return *t;
}

actions::proto::Strategy* Unit:: mutable_strategy() {
  return proto_.mutable_strategy();
}

actions::proto::Plan* Unit::mutable_plan() {
  return proto_.mutable_plan();
}

const actions::proto::Strategy& Unit:: strategy() {
  return proto_.strategy();
}

const actions::proto::Plan& Unit::plan() {
  return proto_.plan();
}

uint64 Unit::speed_u(geography::proto::ConnectionType type) const {
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

Unit::Unit(const proto::Unit& proto) : proto_(proto) {
  units_[proto_.unit_id()] = this;
}

Unit::~Unit() {
  units_.erase(proto_.unit_id());
}

market::Measure Unit::Capacity(const std::string& good) const {
  if (market::TransportType(good) == market::proto::TradeGood::TTT_IMMOBILE) {
    return 0;
  }
  market::Measure current_bulk_u = 0;
  market::Measure current_weight_u = 0;
  for (const auto& quantity : resources().quantities()) {
    current_bulk_u +=
        micro::MultiplyU(market::BulkU(quantity.first), quantity.second);
    current_weight_u +=
        micro::MultiplyU(market::WeightU(quantity.first), quantity.second);
  }

  // TODO: Scaling effects - also elsewhere!
  // TODO: Maybe this had better be cached?
  market::Measure remaining_bulk_u = Template().mobility().max_bulk_u();
  market::Measure remaining_weight_u = Template().mobility().max_weight_u();

  remaining_bulk_u -= current_bulk_u;
  remaining_weight_u -= current_weight_u;

  // Bulk and weight guaranteed nonzero.
  remaining_bulk_u = micro::DivideU(remaining_bulk_u, market::BulkU(good));
  remaining_weight_u = micro::DivideU(remaining_weight_u, market::WeightU(good));

  return std::min(remaining_bulk_u, remaining_weight_u);
}



} // namespace units
