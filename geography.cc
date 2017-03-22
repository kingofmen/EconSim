#include "geography/geography.h"

#include <vector>

#include "industry/proto/industry.pb.h"
#include "market/goods_utils.h"
#include "market/proto/goods.pb.h"

namespace geography {

bool HasFixedCapital(const proto::Field& field,
    const industry::proto::Production &production) {
  for (const auto &step : production.steps()) {
    bool can_do_step = false;
    for (const auto &variant : step.variants()) {
      if (field.fixed_capital() < variant.fixed_capital()) {
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

bool HasLandType(const proto::Field& field, const industry::proto::Production &production) {
  return field.land_type() == production.land_type();
}

bool HasRawMaterials(const proto::Field& field, 
    const industry::proto::Production &production) {
  for (const auto &step : production.steps()) {
    bool can_do_step = false;
    for (const auto &variant : step.variants()) {
      if (field.resources() < variant.raw_materials()) {
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

void Area::Update() {
  market::proto::Quantity temp;
  for (auto& field : *(mutable_fields())) {
    const auto &recovery = field.has_production()
                               ? limits().recovery()
                               : limits().fallow_recovery();
    auto& resources = *field.mutable_resources();
    for (const auto &quantity : recovery.quantities()) {
      //std::cout << "Recovering " << quantity.first << " " << quantity.second;
      temp.set_kind(quantity.first);
      resources >> temp;
      temp += quantity.second.amount();
      temp.set_amount(std::min(
          temp.amount(), market::GetAmount(limits().maximum(), temp)));
      resources << temp;
    }
  }
}

} // namespace geography
