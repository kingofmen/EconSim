// Implementation of production chains.
#include "industry.h"

#include "market/goods_utils.h"

namespace industry {
using market::proto::Container;

void Progress::PerformStep(Container *inputs, Container *output,
                             int variant_index) {
  if (Complete()) {
    return;
  }

  const auto& needed = production_->steps(step()).variants(variant_index);
  if (*inputs < needed.consumables() + needed.movable_capital()) {
    return;
  }

  // TODO: Check fixed_capital.

  *inputs -= needed.consumables();
  set_step(1 + step());
  if (Complete()) {
    *output += production_->outputs();
  }
}

bool Progress::Complete() const {
  return step() >= production_->steps_size();
}

} // namespace industry
