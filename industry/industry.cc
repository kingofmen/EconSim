// Implementation of production chains.
#include "industry.h"

#include <cmath>

#include "market/goods_utils.h"
#include "util/arithmetic/microunits.h"
#include "util/headers/int_types.h"

namespace industry {
using market::proto::Container;

namespace {

// Returns the maximum scaling in micro-units.
int64 MaxScalingU(const proto::Production& production) {
  return (1 + production.scaling_effects_u_size()) * micro::kOneInU;
}

} // namespace

proto::Progress Production::MakeProgress(market::Measure scale_u) const {
  proto::Progress progress;
  progress.set_name(proto_.name());
  progress.set_step(0);
  progress.set_efficiency_u(micro::kOneInU);
  progress.set_scaling_u(std::min(scale_u, MaxScalingU(proto_)));
  return progress;
}

bool Production::Complete(const proto::Progress& progress) const {
  if (proto_.name() != progress.name()) {
    return false;
  }
  return progress.step() >= proto_.steps_size();
}

market::Measure Production::EfficiencyU(const proto::Progress& progress) const {
  market::Measure effect_u = micro::kOneInU;
  const market::Measure scaling_u = progress.scaling_u();
  if (scaling_u <= micro::kOneInU) {
    effect_u = micro::SqrtU(scaling_u);
  } else {
    int last_full_step = 1;
    for (; (last_full_step + 1) * micro::kOneInU < scaling_u;
         ++last_full_step) {
      effect_u += proto_.scaling_effects_u(last_full_step - 1);
    }
    int64 fractional = scaling_u - last_full_step * micro::kOneInU;
    if (fractional > 0) {
      effect_u +=
          micro::MultiplyU(micro::SqrtU(fractional),
                           proto_.scaling_effects_u(last_full_step - 1));
    }
  }
  return micro::MultiplyU(effect_u, progress.efficiency_u());
}

market::proto::Container
Production::ExpectedOutput(const proto::Progress& progress) const {
  auto output = proto_.outputs();
  micro::MultiplyU(output, EfficiencyU(progress));
  return output;
}

market::Measure Production::ExperienceEffectU(
    const market::Measure institutional_capital_u) const {
  return micro::kOneInU -
         micro::MultiplyU(institutional_capital_u,
                          proto_.experience_effect_u());
}

market::Measure Production::MaxScaleU() const {
  return (1 + proto_.scaling_effects_u_size()) * micro::kOneInU;
}

void Production::PerformStep(const Container& fixed_capital,
                             const market::Measure institutional_capital_u,
                             const int variant_index, Container* inputs,
                             Container* raw_materials, Container* output,
                             proto::Progress* progress) const {
  if (proto_.name() != progress->name()) {
    return;
  }
  if (Complete(*progress)) {
    return;
  }

  const int step = progress->step();
  const auto& needed = proto_.steps(step).variants(variant_index);
  if (!(fixed_capital > needed.fixed_capital())) {
    return;
  }

  const market::Measure experience_u =
      ExperienceEffectU(institutional_capital_u);
  const market::Measure scaling_u = progress->scaling_u();
  auto needed_raw_material = needed.raw_materials();
  micro::MultiplyU(needed_raw_material,
                  micro::MultiplyU(scaling_u, experience_u));

  if (!(*raw_materials > needed_raw_material)) {
    return;
  }

  auto required = RequiredConsumables(*progress, variant_index);
  micro::MultiplyU(required, experience_u);

  if (!(*inputs > required)) {
    return;
  }

  // TODO: Weather and other adverse effects.

  *raw_materials -= needed_raw_material;
  auto used = needed.consumables();
  micro::MultiplyU(used, micro::MultiplyU(scaling_u, experience_u));
  *inputs -= used;
  progress->set_step(1 + step);
  if (Complete(*progress)) {
    *output += ExpectedOutput(*progress);
  }
}

market::proto::Container
Production::RequiredConsumables(const int step, const int variant) const {
  market::proto::Container consumables;
  const auto& input = proto_.steps(step).variants(variant);
  consumables += input.consumables();
  consumables += input.movable_capital();
  return consumables;
}

market::proto::Container
Production::RequiredConsumables(const proto::Progress& progress,
                                const int variant) const {
  if (proto_.name() != progress.name()) {
    return market::proto::Container();
  }
  if (Complete(progress)) {
    return market::proto::Container();
  }
  auto required = RequiredConsumables(progress.step(), variant);
  micro::MultiplyU(required, progress.scaling_u());
  return required;
}

void Production::Skip(proto::Progress* progress) const {
  if (Complete(*progress)) {
    return;
  }
  const int step = progress->step();
  progress->set_efficiency_u(micro::MultiplyU(
      progress->efficiency_u(), proto_.steps(step).skip_effect_u()));
  progress->set_step(1 + step);
}

} // namespace industry
