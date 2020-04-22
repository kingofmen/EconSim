// Implementation of production chains.
#include "games/industry/industry.h"

#include <cmath>

#include "games/market/goods_utils.h"
#include "util/arithmetic/microunits.h"

namespace industry {
using market::proto::Container;

namespace {} // namespace

proto::Progress Production::MakeProgress(micro::Measure scale_u) const {
  proto::Progress progress;
  progress.set_name(proto_.name());
  progress.set_step(0);
  progress.set_efficiency_u(micro::kOneInU);
  progress.set_scaling_u(std::min(scale_u, MaxScaleU()));
  return progress;
}

bool Production::Complete(const proto::Progress& progress) const {
  if (proto_.name() != progress.name()) {
    return false;
  }
  return progress.step() >= proto_.steps_size();
}

micro::Measure Production::EfficiencyU(const proto::Progress& progress) const {
  micro::Measure effect_u = micro::kOneInU;
  const micro::Measure scaling_u = progress.scaling_u();
  if (scaling_u <= micro::kOneInU) {
    effect_u = scaling_u;
  } else {
    micro::Measure acc_u = micro::kOneInU;
    for (const auto& s : proto_.scaling()) {
      if (acc_u + s.size_u() < scaling_u) {
        acc_u += s.size_u();
        effect_u += s.effect_u();
        continue;
      }
      micro::Measure frac_u = micro::DivideU(scaling_u - acc_u, s.size_u());
      effect_u += micro::MultiplyU(frac_u, s.effect_u());
      break;
    }
  }
  return micro::MultiplyU(effect_u, progress.efficiency_u());
}

market::proto::Container
Production::ExpectedOutput(const proto::Progress& progress) const {
  auto output = proto_.outputs();
  market::MultiplyU(output, EfficiencyU(progress));
  return output;
}

micro::Measure Production::ExperienceEffectU(
    const micro::Measure institutional_capital_u) const {
  return micro::kOneInU -
         micro::MultiplyU(institutional_capital_u,
                          proto_.experience_effect_u());
}

micro::Measure Production::MaxScaleU() const {
  micro::Measure ret = micro::kOneInU;
  for (const auto& s : proto_.scaling()) {
    ret += s.size_u();
  }
  return ret;
}

bool Production::StepPossible(
    const Container& fixed_capital, const Container& inputs,
    const Container& raw_materials, const proto::Progress& progress,
    const micro::Measure institutional_capital_u, const int variant_index,
    Container* needed_capital, Container* needed_inputs,
    Container* needed_raw_material) const {
  if (proto_.name() != progress.name()) {
    return false;
  }
  if (Complete(progress)) {
    return false;
  }

  const int step = progress.step();
  const micro::Measure scaling_u = progress.scaling_u();
  const auto& step_inputs = proto_.steps(step).variants(variant_index);
  auto required_fixcap = step_inputs.fixed_capital();
  market::MultiplyU(required_fixcap, scaling_u);

  if (!(fixed_capital > required_fixcap)) {
    return false;
  }

  const micro::Measure experience_u =
      ExperienceEffectU(institutional_capital_u);
  *needed_raw_material = step_inputs.raw_materials();
  market::MultiplyU(*needed_raw_material,
                   micro::MultiplyU(scaling_u, experience_u));

  if (!(raw_materials > *needed_raw_material)) {
    return false;
  }

  *needed_inputs = step_inputs.consumables();
  market::MultiplyU(*needed_inputs, micro::MultiplyU(scaling_u, experience_u));
  *needed_capital = step_inputs.movable_capital();
  market::MultiplyU(*needed_capital, scaling_u);
  auto required = *needed_inputs + *needed_capital;

  return inputs > required;
}

bool Production::PerformStep(const Container& fixed_capital,
                             const micro::Measure institutional_capital_u,
                             const int variant_index, Container* inputs,
                             Container* raw_materials, Container* output,
                             Container* used_capital,
                             proto::Progress* progress) const {
  Container needed_capital;
  Container needed_inputs;
  Container needed_raw_material;
  if (!StepPossible(fixed_capital, *inputs, *raw_materials, *progress,
                    institutional_capital_u, variant_index, &needed_capital,
                    &needed_inputs, &needed_raw_material)) {
    return false;
  }

  // TODO: Weather and other adverse effects.

  *raw_materials -= needed_raw_material;
  *inputs -= needed_inputs;
  *inputs -= needed_capital;
  *used_capital << needed_capital;

  progress->set_step(1 + progress->step());
  if (Complete(*progress)) {
    *output += ExpectedOutput(*progress);
  }
  return true;
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
  market::MultiplyU(required, progress.scaling_u());
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
