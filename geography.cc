#include "geography/geography.h"

#include <vector>

#include "industry/proto/industry.pb.h"
#include "market/goods_utils.h"

namespace geography {

void Field::PossibleProductionChains(
    const std::vector<const industry::proto::Production *> &candidates,
    std::vector<const industry::proto::Production *> *possible) const {
  for (const auto *candidate : candidates) {
    if (candidate->land_type() != land_type()) {
      continue;
    }
    bool can_do_all_steps = true;
    for (const auto& step : candidate->steps()) {
      bool can_do_step = false;
      for (const auto& variant : step.variants()) {
        if (fixed_capital() < variant.fixed_capital()) {
          continue;
        }
        can_do_step = true;
        break;
      }
      if (can_do_step) {
        continue;
      }
      can_do_all_steps = false;
      break;
    }
    if (!can_do_all_steps) {
      continue;
    }
    possible->push_back(candidate);
  }
}

} // namespace geography
