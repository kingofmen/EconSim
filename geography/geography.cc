#include "geography/geography.h"

#include <cmath>
#include <limits>
#include <vector>

#include "industry/proto/industry.pb.h"
#include "market/goods_utils.h"
#include "market/proto/goods.pb.h"
#include "util/status/status.h"

namespace geography {

std::unordered_map<uint64, Area*> Area::id_map_;
uint64 Area::max_id_ = 0;
uint64 Area::min_id_ = std::numeric_limits<uint64>::max();

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
  if (area.id() == 0) {
    // TODO: Actually handle some errors here and below.
    return ret;
  }
  if (id_map_.find(area.id()) != id_map_.end()) {
    return ret;
  }
  ret.reset(new Area(area));
  return ret;
}

Area::Area(const proto::Area& area) : proto_(area), market_(area.market()) {
  if (proto_.id() == 0) {
    // TODO: Handle error here.
  }
  id_map_[id()] = this;
  if (id() > max_id_) {
    max_id_ = id();
  }
  if (id() < min_id_) {
    min_id_ = id();
  }
}

Area::~Area() {
  uint64 erased = id();
  id_map_.erase(erased);
  if (erased == max_id_ || erased == min_id_) {
    uint64 least = std::numeric_limits<uint64>::max();
    uint64 most = 0;
    for (const auto& area : id_map_) {
      if (area.first > most) {
        most = area.first;
      }
      if (area.first < least) {
        least = area.first;
      }
    }
    max_id_ = most;
    min_id_ = least;
  }
}

Area* Area::GetById(uint64 id) {
  return id_map_[id];
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

} // namespace geography
