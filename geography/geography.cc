#include "geography/geography.h"

#include <cmath>
#include <limits>
#include <vector>

#include "industry/proto/industry.pb.h"
#include "market/goods_utils.h"
#include "market/proto/goods.pb.h"
#include "util/logging/logging.h"
#include "util/proto/object_id.h"
#include "util/status/status.h"

std::unordered_map<util::proto::ObjectId, geography::Area*> area_id_map_;

namespace geography {

util::proto::ObjectId Area::max_id_ = util::objectid::kNullId;
util::proto::ObjectId Area::min_id_ = util::objectid::kNullId;

bool HasFixedCapital(const proto::Field& field,
                     const industry::Production& production) {
  const auto* proto = production.Proto();
  for (const auto& step : proto->steps()) {
    bool can_do_step = false;
    for (const auto& variant : step.variants()) {
      if (!(field.fixed_capital() > variant.fixed_capital())) {
        continue;
      }
      can_do_step = true;
      break;
    }
    if (!can_do_step) {
      return false;
    }
  }
  return true;
}

bool HasLandType(const proto::Field& field,
                 const industry::Production& production) {
  const auto* proto = production.Proto();
  return field.land_type() == proto->land_type();
}

bool HasRawMaterials(const proto::Field& field,
                     const industry::Production& production) {
  const auto* proto = production.Proto();
  for (const auto& step : proto->steps()) {
    bool can_do_step = false;
    for (const auto& variant : step.variants()) {
      if (!(field.resources() > variant.raw_materials())) {
        continue;
      }
      can_do_step = true;
      break;
    }
    if (!can_do_step) {
      return false;
    }
  }
  return true;
}

google::protobuf::util::Status
GenerateTransitionProcess(const proto::Field& field,
                          const proto::Transition& transition,
                          industry::Production* production) {
  auto* proto = production->Proto();
  if (field.land_type() != transition.source()) {
    return util::InvalidArgumentError("Invalid land type");
  }
  int total_steps = transition.steps();
  int max_fixed_cap_steps = 0;
  for (const auto& fix_cap : transition.step_fixed_capital().quantities()) {
    const auto& resource = fix_cap.first;
    auto field_amount = market::GetAmount(field.fixed_capital(), resource);
    auto step_amount = fix_cap.second;
    int curr_steps = (int)ceil(field_amount / step_amount);
    max_fixed_cap_steps = std::max(curr_steps, max_fixed_cap_steps);
  }
  total_steps += max_fixed_cap_steps;
  for (int i = 0; i < total_steps; ++i) {
    auto* step = proto->add_steps();
    auto* variant = step->add_variants();
    auto& input = *variant->mutable_consumables();
    input += transition.step_input();
  }
  return util::OkStatus();
}

std::unique_ptr<Area> Area::FromProto(const proto::Area& area) {
  std::unique_ptr<Area> ret;
  // TODO: Actually handle some errors here and below.
  if (!area.has_area_id()) {
    Log::Errorf("Area %s has no area_id", area.DebugString());
    return ret;
  }

  if (area.area_id().number() == 0) {
    Log::Errorf("Invalid area id 0: %s", area.DebugString());
    return ret;
  }
  if (area_id_map_.find(area.area_id()) != area_id_map_.end()) {
    Log::Errorf("Area %s already exists", area.area_id().DebugString());
    return ret;
  }
  ret.reset(new Area(area));
  return ret;
}

Area::Area(const proto::Area& area) : proto_(area), market_(area.market()) {
  area_id_map_[proto_.area_id()] = this;
  uint64 number = proto_.area_id().number();
  if (number > max_id_.number() || min_id_ == util::objectid::kNullId) {
    max_id_ = proto_.area_id();
  }
  if (number < min_id_.number() || min_id_ == util::objectid::kNullId) {
    min_id_ = proto_.area_id();
  }
}

Area::~Area() {
  uint64 erased = proto_.area_id().number();
  area_id_map_.erase(proto_.area_id());

  if (erased == max_id_.number() || erased == min_id_.number()) {
    uint64 least = std::numeric_limits<uint64>::max();
    uint64 most = 0;
    for (const auto& area : area_id_map_) {
      uint64 number = area.first.number();
      if (number > most) {
        most = number;
      }
      if (number < least) {
        least = number;
      }
    }
    max_id_.set_number(most);
    min_id_.set_number(least);
  }
}

const util::proto::ObjectId& Area::area_id() const {
  return proto_.area_id();
}

Area* Area::GetById(const util::proto::ObjectId& area_id) {
  if (area_id_map_.find(area_id) == area_id_map_.end()) {
    return nullptr;
  }
  return area_id_map_.at(area_id);
}

void Area::Update() {
  market::proto::Quantity temp;
  // Recovery of raw materials in the fields, eg topsoil.
  for (auto& field : *(proto_.mutable_fields())) {
    const auto& recovery = field.has_progress()
                               ? proto_.limits().recovery()
                               : proto_.limits().fallow_recovery();
    auto& resources = *field.mutable_resources();
    for (const auto& quantity : recovery.quantities()) {
      temp.set_kind(quantity.first);
      resources >> temp;
      temp += quantity.second;
      temp.set_amount(std::min(
          temp.amount(), market::GetAmount(proto_.limits().maximum(), temp)));
      resources << temp;
    }
  }
}

const std::vector<uint64> Area::pop_ids() const {
  return std::vector<uint64>(proto_.pop_ids().begin(), proto_.pop_ids().end());
}

const std::vector<const proto::Field*> Area::fields() const {
  std::vector<const proto::Field*> ret;
  for (const auto& field : proto_.fields()) {
    ret.push_back(&field);
  }
  return ret;
}

} // namespace geography
