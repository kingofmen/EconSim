// Implementation of production chains.
#include "industry.h"

#include <cmath>

#include "market/goods_utils.h"

#include <iostream>

namespace industry {
using market::proto::Container;

Progress::Progress(const proto::Production *prod, double scale)
    : production_(prod) {
  set_name(prod->name());
  set_step(0);
  set_efficiency(1.0);
  if (scale > 1 + prod->scaling_effects_size()) {
    scale = 1 + prod->scaling_effects_size();
  }
  set_scaling(scale);
}

double Progress::ExperienceEffect(const double institutional_capital) const {
  return 1.0 - institutional_capital * production_->experience_effect();
}

void Progress::PerformStep(const Container &fixed_capital, Container *inputs,
                           Container *raw_materials, Container *output,
                           const double institutional_capital,
                           const int variant_index) {
  if (Complete()) {
    return;
  }

  const auto &needed = production_->steps(step()).variants(variant_index);
  if (!(fixed_capital > needed.fixed_capital())) {
    return;
  }

  double experience = ExperienceEffect(institutional_capital);
  auto needed_raw_material = needed.raw_materials() * scaling() * experience;
  if (!(*raw_materials > needed_raw_material)) {
    return;
  }

  auto required = needed.consumables() + needed.movable_capital();
  required *= scaling();
  required *= experience;
  if (!(*inputs > required)) {
    return;
  }

  // TODO: Weather and other adverse effects.

  *raw_materials -= needed_raw_material;
  *inputs -= needed.consumables() * scaling() * experience;
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
  double effect = 1;
  if (scaling() <= 1) {
    effect = sqrt(scaling());
  } else {
    int last_full_step = 1;
    for (; last_full_step + 1 < scaling(); ++last_full_step) {
      effect += production_->scaling_effects(last_full_step - 1);
    }
    if (scaling() - last_full_step > 0) {
      effect += sqrt(scaling() - last_full_step) *
                production_->scaling_effects(last_full_step - 1);
    }
  }
  effect *= efficiency();
  return effect;
}

bool Progress::Complete() const { return step() >= production_->steps_size(); }

} // namespace industry
