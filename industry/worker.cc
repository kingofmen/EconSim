#include "industry/worker.h"

#include "absl/strings/substitute.h"
#include "market/goods_utils.h"
#include "util/arithmetic/microunits.h"

namespace industry {
namespace {

// Returns the unit cost of input, with prices estimated by market at
// stepsAhead.
market::Measure CalculateUnitCostU(const proto::Input& input,
                             const market::PriceEstimator& market,
                             unsigned int stepsAhead) {
  // TODO: Opportunity cost of movable capital?
  return market.GetPriceU(input.consumables(), stepsAhead);
}

// Returns the unit cost of capex, with prices estimated by market at
// stepsAhead.
market::Measure CalculateCapCostU(const proto::Input& input,
                            const market::PriceEstimator& market,
                            unsigned int stepsAhead,
                            const market::proto::Container& fixcap) {
  auto required = market::SubtractFloor(input.fixed_capital(), fixcap);
  return market.GetPriceU(required, stepsAhead);
}

}  // namespace

void CalculateProductionScale(const market::proto::Container& wealth,
                              decisions::ProductionContext* context,
                              geography::proto::Field* field) {
  std::vector<decisions::proto::ProductionInfo>& cands =
      context->candidates.at(field);
  for (decisions::proto::ProductionInfo& cand : cands) {
    const Production* chain = context->production_map->at(cand.name());
    auto overall_scale = chain->MaxScaleU();
    proto::Progress progress;
    if (field->has_progress() && field->progress().name() == cand.name()) {
      progress = field->progress();
    } else {
      progress = chain->MakeProgress(overall_scale);
    }
    for (unsigned int step = progress.step(); step < chain->num_steps();
         ++step) {
      auto ahead = step - progress.step();
      if (ahead >= cand.step_info_size()) {
        // This should never happen.
        break;
      }

      decisions::proto::StepInfo* step_info = cand.mutable_step_info(ahead);
      const proto::ProductionStep& prod_step = chain->get_step(step);
      market::Measure step_scale = 0;
      for (int var = 0; var < step_info->variant_size(); ++var) {
        const proto::Input& input = prod_step.variants(var);
        decisions::proto::VariantInfo* var_info =
            step_info->mutable_variant(var);
        auto variant_scale_u = overall_scale;
        // TODO: Account for installation costs.
        const auto& fixcap = input.fixed_capital();
        for (const auto& good : fixcap.quantities()) {
          market::Measure ratio_u = micro::DivideU(
              (context->market->Available(good.first, ahead) +
               market::GetAmount(field->fixed_capital(), good.first) +
               market::GetAmount(wealth, good.first)),
              good.second);
          if (ratio_u < variant_scale_u) {
            variant_scale_u = ratio_u;
            var_info->set_bottleneck(absl::Substitute("FixCap $0", good.first));
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
        auto consumed = input.consumables();
        const auto& movable = input.movable_capital();
        auto required_per_unit = consumed + movable;
        for (const auto& good : required_per_unit.quantities()) {
          market::Measure ratio_u = micro::DivideU(
              (context->market->AvailableImmediately(good.first) +
               market::GetAmount(wealth, good.first)),
              good.second);
          if (ratio_u < variant_scale_u) {
            variant_scale_u = ratio_u;
            var_info->set_bottleneck(
                absl::Substitute("Consumable $0", good.first));
          }
        }

        var_info->set_possible_scale_u(variant_scale_u);
        if (variant_scale_u > step_scale) {
          step_scale = variant_scale_u;
        }
      }
      if (step_scale < overall_scale) {
        overall_scale = step_scale;
      }
    }

    cand.set_max_scale_u(overall_scale);
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
      var_info->set_cap_cost_u(
          CalculateCapCostU(input, prices, ahead, field.fixed_capital()));
    }
  }
}

void SelectProduction(const decisions::ProductionEvaluator& evaluator,
                      decisions::ProductionContext* context,
                      geography::proto::Field* field) {
  auto& entry = context->decisions->find(field);
  if (entry == context->decisions->end()) {
    // This should never happen.
    return;
  }
  decisions::proto::ProductionDecision& decision = entry->second;
  auto& cands = context->candidates.find(field);
  if (cands == context->candidates.end()) {
    // This, too, should never happen.
    return;
  }

  evaluator.SelectCandidate(*context, cands->second, &decision);
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

  production.PerformStep(field->fixed_capital(), 0, variant_index, source,
                         field->mutable_resources(), target, used_capital,
                         progress);

  if (production.Complete(*progress)) {
    field->clear_progress();
  }

  return true;
}

}  // namespace industry
