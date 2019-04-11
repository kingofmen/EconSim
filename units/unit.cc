#include "units/unit.h"

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
  return &templates_[id];
}

uint64 Unit::speed_u(geography::proto::ConnectionType type) const {
  return 1;
}

Unit::Unit(const proto::Unit& proto) : proto_(proto) {}
Unit::~Unit() {}



} // namespace units
