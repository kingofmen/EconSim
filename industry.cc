// Implementation of production chains.
#include "industry.h"

#include "market/goods_utils.h"

namespace industry {
using market::proto::Container;

Progress::Progress(const proto::Production *prod, const int scale)
    : production_(prod) {
  set_name(prod->name());
  set_step(0);
  set_efficiency(1.0);
  set_scaling(scale);
}

void Progress::PerformStep(Container *inputs, Container *output,
                           int variant_index) {
  if (Complete()) {
    return;
  }

  const auto &needed = production_->steps(step()).variants(variant_index);
  auto required = needed.consumables() + needed.movable_capital();
  required *= scaling();
  if (*inputs < required) {
    return;
  }

  // TODO: Check fixed_capital.
  // TODO: Weather and other adverse effects.

  *inputs -= needed.consumables() * scaling();
  set_step(1 + step());
  if (Complete()) {
    *output += production_->outputs() * Efficiency();
  }
}

void Progress::Skip() {
  if (Complete()) {
    return;
  }
  set_efficiency(efficiency() * production_->steps(step()).skip_effect());
  set_step(1 + step());
}

double Progress::Efficiency() const {
  double effect = efficiency();
  if (production_->scaling_effects_size() > 0) {
    effect *= production_->scaling_effects(scaling() - 1);
  }
  return effect;
}

bool Progress::Complete() const { return step() >= production_->steps_size(); }

} // namespace industry
