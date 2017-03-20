#include "geography/geography.h"

#include <vector>

#include "industry/proto/industry.pb.h"
#include "market/goods_utils.h"

namespace geography {

bool Field::HasFixedCapital(
    const industry::proto::Production &production) const {
  for (const auto &step : production.steps()) {
    bool can_do_step = false;
    for (const auto &variant : step.variants()) {
      if (fixed_capital() < variant.fixed_capital()) {
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

bool Field::HasLandType(const industry::proto::Production &production) const {
  return land_type() == production.land_type();
}

bool Field::HasRawMaterials(
    const industry::proto::Production &production) const {
  for (const auto &step : production.steps()) {
    bool can_do_step = false;
    for (const auto &variant : step.variants()) {
      if (resources() < variant.raw_materials()) {
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

} // namespace geography
