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


int Production::CheapestVariant(const market::Market& market,
                                const market::proto::Container& existing,
                                const market::proto::Container& capital,
                                const int step,
                                double* price) const {
  double least_expense = std::numeric_limits<double>::max();
  int cheapest_variant = -1;
  for (int index = 0; index < steps(step).variants_size(); ++index) {
    auto consumables = RequiredConsumables(step, index);
    bool allAvailable = true;
    if (!(existing > consumables)) {
      for (const auto& consumable : consumables.quantities()) {
        if (market.AvailableImmediately(consumable.first) <
            consumable.second - market::GetAmount(existing, consumable.first)) {
          allAvailable = false;
          break;
        }
      }
    }
    if (!allAvailable) {
      continue;
    }
    const auto& variant = steps(step).variants(index);
    if (!(capital > variant.fixed_capital())) {
      continue;
    }
    double variant_expense = consumables * market.prices();
    if (variant_expense < least_expense) {
      cheapest_variant = index;
      least_expense = variant_expense;
    }
  }
  *price = least_expense;
  return cheapest_variant;
}

double Production::ExpectedProfit(const market::Market& market,
                                  const market::proto::Container& existing,
                                  const market::proto::Container& capital,
                                  const proto::Progress* progress) const {
  double expense = 0;
  int step = 0;
  double efficiency = 1;
  double scaling = 1;
  double experience = 1;
  if (progress != nullptr) {
    step = progress->step();
    scaling = progress->scaling();
    efficiency = Efficiency(*progress);
  }
  double least_expense = 0;
  for (; step < steps_size(); ++step) {
    int index = CheapestVariant(market, existing, capital, step, &least_expense);
    if (index == -1) {
      return -1;
    }
    expense += least_expense;
  }
  expense *= scaling;
  expense *= experience;
  double revenue = outputs() * market.prices() * efficiency;
  return revenue - expense;
}

double Production::ExperienceEffect(const double institutional_capital) const {
  return 1.0 - institutional_capital * experience_effect();
}

bool Production::GoodsForVariantAvailable(
    const market::Market& market, const market::proto::Container& existing,
    const int step, const int variant_index) const {
  if (step >= steps_size()) {
    return false;
  }
  if (variant_index >= steps(step).variants_size()) {
    return false;
  }
  const auto needed = RequiredConsumables(step, variant_index);
  if (existing > needed) {
    return true;
  }
  for (const auto& good : needed.quantities()) {
    double amount = good.second;
    amount -= market::GetAmount(existing, good.first);
    if (market.AvailableImmediately(good.first) < amount) {
      return false;
    }
  }
  return true;
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
    *output += outputs() * Efficiency(*progress);
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
