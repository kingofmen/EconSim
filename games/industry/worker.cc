#include "industry/worker.h"

#include "absl/strings/substitute.h"
#include "market/goods_utils.h"
#include "util/arithmetic/microunits.h"
#include "util/logging/logging.h"

namespace industry {
namespace {

// Returns the number of unit_u that can be made from available_u.
market::Measure AvailableUnits(market::Measure available_u,
                               market::Measure unit_u) {
  if (unit_u == 0) {
    return micro::kMaxU;
  }
  if (available_u == 0) {
    return 0;
  }
  uint64 overflow = 0;
  market::Measure units_u = micro::DivideU(available_u, unit_u, &overflow);
  if (overflow != 0) {
    return micro::kMaxU;
  }
  return units_u;
}

// Returns the unit cost of input, with prices estimated by market at
// stepsAhead.
market::Measure CalculateUnitCostU(const proto::Input& input,
                                   const market::PriceEstimator& market,
                                   unsigned int stepsAhead) {
  // TODO: Opportunity cost of movable capital?
  return market.GetPriceU(input.consumables(), stepsAhead);
}

// Returns the scale cost of capex, with prices estimated by market at
// stepsAhead.
market::Measure CalculateCapCostU(const proto::Input& input,
                                  const market::PriceEstimator& market,
                                  market::Measure scale_u,
                                  unsigned int stepsAhead,
                                  const market::proto::Container& fixcap) {
  market::proto::Container to_install = input.fixed_capital();
  micro::MultiplyU(to_install, scale_u);
  to_install = market::SubtractFloor(to_install, fixcap, 0);
  market::CleanContainer(&to_install);
  market::Measure least_install_scale_u = scale_u;
  for (const auto& good : to_install.quantities()) {
    market::Measure install_scale_u =
        AvailableUnits(market::GetAmount(fixcap, good.first),
                       market::GetAmount(input.fixed_capital(), good.first));
    if (install_scale_u < least_install_scale_u) {
      least_install_scale_u = install_scale_u;
    }
  }
  market::proto::Container install_cost = input.install_cost();
  micro::MultiplyU(install_cost, scale_u - least_install_scale_u);

  to_install += install_cost;
  return market.GetPriceU(to_install, stepsAhead);
}

// Returns the possible scale if resource is the bottleneck. The scale s is the
// solution to a system of equations:
// Installed (c) plus existing (Ce) capital, divided by capital per unit (C),
// equals scale:
// s = (c + Ce)/C
// Install cost i over unit install cost I equals installed capital over unit
// capital:
// i/I = c/C
// (or alternatively, number of installed units, c/C, times install cost per
// unit, I, equals total install cost i).
// Consumed u over unit consumption L equals scale,
// u/L = s
// Installed capital, plus install cost, plus consumed, equals available A:
// c + i + u = A
// Cases where C, L, or I are zero are treated specially. For resources that
// are only used in installation, the existing installed cost Ie, that is, the
// cost in install-resources of the existing capital base, is used.
market::Measure CalculatePossibleScale(market::Measure available_A_u,
                                       market::Measure capital_unit_C_u,
                                       market::Measure consumed_unit_L_u,
                                       market::Measure install_unit_I_u,
                                       market::Measure existing_Ce_u,
                                       market::Measure existing_Ie_u) {
  if (capital_unit_C_u + consumed_unit_L_u + install_unit_I_u == 0) {
    return micro::kMaxU;
  }
  uint64 overflow = 0;
  market::Measure scale_u = 0;

  if (consumed_unit_L_u + install_unit_I_u == 0) {
    // This resource is only used for fixed capital.
    return AvailableUnits(available_A_u + existing_Ce_u, capital_unit_C_u);
  }

  if (capital_unit_C_u + install_unit_I_u == 0) {
    // This resource is only used for consumption.
    return AvailableUnits(available_A_u, consumed_unit_L_u);
  }

  if (capital_unit_C_u + consumed_unit_L_u == 0) {
    // This resource only appears in installation costs.
    return AvailableUnits(available_A_u + existing_Ie_u, install_unit_I_u);
  }

  if (capital_unit_C_u == 0) {
    return AvailableUnits(available_A_u + existing_Ie_u,
                          consumed_unit_L_u + install_unit_I_u);
  }

  if (install_unit_I_u == 0) {
    return AvailableUnits(available_A_u + existing_Ce_u,
                          capital_unit_C_u + consumed_unit_L_u);
  }

  if (consumed_unit_L_u == 0) {
    market::Measure c_u = micro::MultiplyU(available_A_u, capital_unit_C_u);
    c_u = AvailableUnits(c_u, install_unit_I_u + capital_unit_C_u);
    c_u += existing_Ce_u;
    return AvailableUnits(c_u, capital_unit_C_u);
  }

  if (available_A_u == 0) {
    return 0;
  }

  market::Measure c_u = micro::MultiplyU(available_A_u, capital_unit_C_u);
  c_u -= micro::MultiplyU(existing_Ce_u, consumed_unit_L_u);
  c_u = micro::DivideU(
      c_u, install_unit_I_u + capital_unit_C_u + consumed_unit_L_u, &overflow);
  if (overflow != 0) {
    // Installable capital is humongous, indicating effectively that there is no
    // limit from this resource.
    return micro::kMaxU;
  }

  overflow = 0;
  scale_u = micro::DivideU(c_u + existing_Ce_u, capital_unit_C_u, &overflow);
  if (overflow != 0) {
    return micro::kMaxU;
  }

  return scale_u;
}

}  // namespace

void CalculateProductionScale(const market::proto::Container& wealth,
                              decisions::ProductionContext* context,
                              geography::proto::Field* field) {
  auto& cands = context->fields.at(field).candidates;
  for (auto& cand : cands) {
    const Production* chain = context->production_map->at(cand->name());
    auto overall_scale = chain->MaxScaleU();
    proto::Progress progress;
    if (field->has_progress() && field->progress().name() == cand->name()) {
      progress = field->progress();
    } else {
      progress = chain->MakeProgress(overall_scale);
    }
    for (unsigned int step = progress.step(); step < chain->num_steps();
         ++step) {
      auto ahead = step - progress.step();
      if (ahead >= cand->step_info_size()) {
        // This should never happen.
        break;
      }

      decisions::proto::StepInfo* step_info = cand->mutable_step_info(ahead);
      const proto::ProductionStep& prod_step = chain->get_step(step);
      market::Measure step_scale = 0;
      for (int var = 0; var < step_info->variant_size(); ++var) {
        const proto::Input& input = prod_step.variants(var);
        decisions::proto::VariantInfo* var_info =
            step_info->mutable_variant(var);
        auto variant_scale_u = overall_scale;
        market::proto::Container fixcap = input.fixed_capital();
        // Movable capital can be treated as "consumed" for purposes of the
        // scale calculation.
        market::proto::Container consumed =
            input.consumables() + input.movable_capital();
        market::proto::Container install = input.install_cost();
        market::proto::Container total = fixcap + consumed + install;

        // Installed capital.
        const market::proto::Container& existing_Ce_u = field->fixed_capital();

        // The cost of installed capital is the cost of the smallest installed
        // resource.
        market::Measure least_capital_scale_u = variant_scale_u;
        for (const auto& good : total.quantities()) {
          market::Measure unit_u = market::GetAmount(fixcap, good.first);
          if (unit_u == 0) {
            continue;
          }
          uint64 overflow = 0;
          market::Measure installed_units_u = micro::DivideU(
              market::GetAmount(existing_Ce_u, good.first), unit_u, &overflow);
          if (overflow != 0) {
            continue;
          }
          if (installed_units_u < least_capital_scale_u) {
            least_capital_scale_u = installed_units_u;
          }
        }
        // Install costs of existing capital.
        market::proto::Container existing_Ie_u = install;
        micro::MultiplyU(existing_Ie_u, least_capital_scale_u);

        for (const auto& good : total.quantities()) {
          market::Measure available_u =
              context->market->Available(good.first, ahead) +
              market::GetAmount(wealth, good.first);
          market::Measure ratio_u = CalculatePossibleScale(
              available_u, market::GetAmount(fixcap, good.first),
              market::GetAmount(consumed, good.first),
              market::GetAmount(install, good.first),
              market::GetAmount(existing_Ce_u, good.first),
              market::GetAmount(existing_Ie_u, good.first));
          if (ratio_u < variant_scale_u) {
            variant_scale_u = ratio_u;
            var_info->set_bottleneck(good.first);
          }
        }
        for (const auto& good : input.raw_materials().quantities()) {
          market::Measure ratio_u = micro::DivideU(
              market::GetAmount(field->resources(), good.first), good.second);
          if (ratio_u < variant_scale_u) {
            variant_scale_u = ratio_u;
            var_info->set_bottleneck(
                absl::Substitute("Resource $0", good.first));
          }
        }

        var_info->set_possible_scale_u(variant_scale_u);
        var_info->set_cap_cost_u(CalculateCapCostU(
            input, *context->market, variant_scale_u, ahead, existing_Ce_u));
        if (variant_scale_u > step_scale) {
          step_scale = variant_scale_u;
        }
      }
      if (step_scale < overall_scale) {
        overall_scale = step_scale;
      }
    }

    cand->set_max_scale_u(overall_scale);
  }
}

void CalculateProductionCosts(const Production& chain,
                              const market::PriceEstimator& prices,
                              const geography::proto::Field& field,
                              decisions::proto::ProductionInfo* production_info) {
  proto::Progress progress;
  if (field.has_progress()) {
    progress = field.progress();
  } else {
    progress = chain.MakeProgress(chain.MaxScaleU());
  }

  *production_info->mutable_expected_output() = chain.ExpectedOutput(progress);
  for (unsigned int step = progress.step(); step < chain.num_steps(); ++step) {
    auto ahead = step - progress.step();
    auto* step_info = production_info->add_step_info();
    const auto& prod_step = chain.get_step(step);
    for (unsigned int var = 0; var < prod_step.variants_size(); ++var) {
      const auto& input = prod_step.variants(var);
      auto* var_info = step_info->add_variant();
      var_info->set_unit_cost_u(CalculateUnitCostU(input, prices, ahead));
    }
  }
}

void SelectProduction(const decisions::ProductionEvaluator& evaluator,
                      decisions::ProductionContext* context,
                      geography::proto::Field* field) {
  evaluator.SelectCandidate(context, field);
}

bool InstallFixedCapital(const proto::Input& production,
                         market::Measure scale_u,
                         market::proto::Container* source,
                         market::proto::Container* target,
                         market::Market* market) {
  // TODO: Should be possible to install X by paying Y, eg housing by paying
  // timber and labour.
  market::proto::Container required = production.fixed_capital();
  micro::MultiplyU(required, scale_u);
  if (*target >= required) {
    return true;
  }
  required = market::SubtractFloor(required, *target, 0);
  market::proto::Container cost = production.install_cost();
  micro::MultiplyU(cost, scale_u);

  market::proto::Container total = required + cost;

  if (total > *source) {
    market::proto::Container to_buy = market::SubtractFloor(total, *source, 0);
    market::CleanContainer(&to_buy);

    if (!market->CanBuy(to_buy, *source)) {
      return false;
    }

    if (!market->BuyBasket(to_buy, source)) {
      return false;
    }
  }

  if (total > *source) {
    return false;
  }

  for (const auto& good : required.quantities()) {
    market::Move(good.first, good.second, source, target);
  }

  *source -= cost;

  return true;
}

// TODO: Don't pass the field here, pass the fixcap and resources. Clear
// separately.
bool TryProductionStep(
    const Production& production, const decisions::proto::StepInfo& step_info,
    geography::proto::Field* field, proto::Progress* progress,
    market::proto::Container* source, market::proto::Container* target,
    market::proto::Container* used_capital, market::Market* market) {

  auto variant_index = step_info.best_variant();
  if (variant_index >= step_info.variant_size()) {
    return false;
  }
  const auto& variant_info = step_info.variant(variant_index);
  if (progress->scaling_u() > variant_info.possible_scale_u()) {
    progress->set_scaling_u(variant_info.possible_scale_u());
  }

  auto required = production.RequiredConsumables(*progress, variant_index);
  required -= *source;
  if (!market->BuyBasket(required, source)) {
    return false;
  }

  if (!production.PerformStep(field->fixed_capital(), 0, variant_index, source,
                              field->mutable_resources(), target, used_capital,
                              progress)) {
    return false;
  }

  if (production.Complete(*progress)) {
    field->clear_progress();
  }

  return true;
}

}  // namespace industry
