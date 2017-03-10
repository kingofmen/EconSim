// Implementation of production chains.
#include "industry.h"

#include "market/goods_utils.h"

namespace industry {
using market::proto::Container;

void Production::PerformStep(Container *inputs, Container *output,
                             int variant_index) {
  if (Complete()) {
    return;
  }

  const auto& needed = steps(current_step_).variants(variant_index);
  if (*inputs < needed.consumables() + needed.movable_capital()) {
    return;
  }

  // TODO: Check fixed_capital.

  *inputs -= needed.consumables();
  ++current_step_;
  if (Complete()) {
    *output += outputs();
  }
}

bool Production::Complete() const {
  return current_step_ >= steps_size();
}

} // namespace industry
