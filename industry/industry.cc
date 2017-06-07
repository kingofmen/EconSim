// Implementation of production chains.
#include "industry.h"

#include <cmath>

#include "market/goods_utils.h"

#include <iostream>

namespace industry {
using market::proto::Container;

proto::Progress Production::MakeProgress(double scale) const {
  proto::Progress progress;
  progress.set_name(name());
  progress.set_step(0);
  progress.set_efficiency(1.0);
  if (scale > 1 + scaling_effects_size()) {
    scale = 1 + scaling_effects_size();
  }
  progress.set_scaling(scale);
  return progress;
}

bool Production::Complete(const proto::Progress& progress) const {
  if (name() != progress.name()) return false;
  return progress.step() >= steps_size();
}

double Production::Efficiency(const proto::Progress& progress) const {
  double effect = 1;
  const double scaling = progress.scaling();
  if (scaling <= 1) {
    effect = sqrt(scaling);
  } else {
    int last_full_step = 1;
    for (; last_full_step + 1 < scaling; ++last_full_step) {
      effect += scaling_effects(last_full_step - 1);
    }
    if (scaling - last_full_step > 0) {
      effect += sqrt(scaling - last_full_step) *
                scaling_effects(last_full_step - 1);
    }
  }
  effect *= progress.efficiency();
  return effect;
}

market::proto::Container
Production::ExpectedOutput(const proto::Progress& progress) const {
  return outputs() * Efficiency(progress);
}

double Production::ExperienceEffect(const double institutional_capital) const {
  return 1.0 - institutional_capital * experience_effect();
}

double Production::MaxScale() const {
  return 1 + scaling_effects_size();
}

void Production::PerformStep(const Container& fixed_capital,
                             const double institutional_capital,
                             const int variant_index, Container* inputs,
                             Container* raw_materials, Container* output,
                             proto::Progress* progress) const {
  if (name() != progress->name()) {
    return;
  }
  if (Complete(*progress)) {
    return;
  }

  const int step = progress->step();
  const auto &needed = steps(step).variants(variant_index);
  if (!(fixed_capital > needed.fixed_capital())) {
    return;
  }

  const double experience = ExperienceEffect(institutional_capital);
  const double scaling = progress->scaling();
  auto needed_raw_material = needed.raw_materials() * scaling * experience;
  if (!(*raw_materials > needed_raw_material)) {
    return;
  }

  auto required = RequiredConsumables(*progress, variant_index);
  required *= experience;
  if (!(*inputs > required)) {
    return;
  }

  // TODO: Weather and other adverse effects.

  *raw_materials -= needed_raw_material;
  *inputs -= needed.consumables() * scaling * experience;
  progress->set_step(1 + step);
  if (Complete(*progress)) {
    *output += ExpectedOutput(*progress);
  }
}

market::proto::Container
Production::RequiredConsumables(const int step, const int variant) const {
  market::proto::Container consumables;
  const auto& input = steps(step).variants(variant);
  consumables += input.consumables();
  consumables += input.movable_capital();
  return consumables;
}

market::proto::Container
Production::RequiredConsumables(const proto::Progress& progress,
                                const int variant) const {
  if (name() != progress.name()) {
    return market::proto::Container();
  }
  if (Complete(progress)) {
    return market::proto::Container();
  }
  return RequiredConsumables(progress.step(), variant) * progress.scaling();
}

void Production::Skip(proto::Progress* progress) const {
  if (Complete(*progress)) {
    return;
  }
  const int step = progress->step();
  progress->set_efficiency(progress->efficiency() * steps(step).skip_effect());
  progress->set_step(1 + step);
}

} // namespace industry
