#include "units/unit.h"

#include <unordered_map>

std::unordered_map<uint64, const units::proto::Template> units::Unit::templates_;
std::unordered_map<util::proto::ObjectId, units::Unit*> units::Unit::units_;

namespace units {

std::unique_ptr<Unit> Unit::FromProto(const proto::Unit& proto) {
  std::unique_ptr<Unit> ret;
  // TODO: Actually handle the errors when we get support for StatusOr<unique_ptr>.
  if (!proto.has_unit_id()) {
    return ret;
  }
  if (!proto.unit_id().has_type()) {
    return ret;
  }
  if (TemplateById(proto.unit_id().type()) == NULL) {
    return ret;
  }
  if (ById(proto.unit_id()) != NULL) {
    return ret;
  }

  ret.reset(new Unit(proto));
  return ret;
}

bool Unit::RegisterTemplate(const proto::Template& proto) {
  if (!proto.has_id()) {
    return false;
  }
  if (TemplateById(proto.id()) != NULL) {
    return false;
  }
  templates_.insert({proto.id(), proto});
  return true;
}

const proto::Template* Unit::TemplateById(uint64 id) {
  if (templates_.find(id) == templates_.end()) {
    return NULL;
  }
  return &templates_[id];
}

Unit* Unit::ById(const util::proto::ObjectId& id) {
  return units_[id];
}

const proto::Template& Unit::Template() const {
  const proto::Template* t = TemplateById(proto_.unit_id().type());
  return *t;
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



} // namespace units
