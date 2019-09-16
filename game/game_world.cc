#include "game/game_world.h"

#include <iostream>

#include "actions/proto/strategy.pb.h"
#include "actions/proto/plan.pb.h"
#include "ai/executer.h"
#include "ai/planner.h"
#include "geography/geography.h"
#include "industry/decisions/production_evaluator.h"
#include "industry/worker.h"
#include "units/unit.h"
#include "util/arithmetic/microunits.h"
#include "util/keywords/keywords.h"

using geography::proto::Field;
using industry::decisions::ProductionContext;

namespace game {
namespace {

class PossibilityFilter : public industry::ProductionFilter {
 public:
  bool Filter(const geography::proto::Field& field, const industry::Production& prod) const {
    if (!geography::HasLandType(field, prod)) {
      return false;
    }
    if (!geography::HasRawMaterials(field, prod)) {
      return false;
    }
    return true;
  }
};

void PrintContainer(const market::proto::Container& container) {
  for (const auto& good : container.quantities()) {
    std::cout << good.first << ":\t" << good.second << "\n";
  }
}

void PrintMarket(const market::proto::MarketProto& market,
                 const market::proto::Container& volumes) {
  std::cout << "Good\tprice\tstored\tvolume\tdebt\n";
  for (const auto& good : market.goods().quantities()) {
    const std::string& name = good.first;
    std::cout << name << "\t" << market::GetAmount(market.prices_u(), name)
              << "\t" << market::GetAmount(market.warehouse(), name) << "\t"
              << market::GetAmount(volumes, name) << "\t"
              << market::GetAmount(market.market_debt(), name) << "\n";
  }
}

// Selects and runs production processes for each field until no fields make
// progress.
void RunAreaIndustry(
    std::unordered_map<population::PopUnit*, ProductionContext>* contexts) {

  std::unordered_set<Field*> progressed;
  std::unordered_map<Field*, int> attempts;
  // TODO: Allow progress to be split between rounds if there is scale loss.
  while (true) {
    geography::proto::Field* best_field = NULL;
    population::PopUnit* best_pop = NULL;
    market::Measure lowest_scale_loss_u = micro::kMaxU;

    for (auto& pop_context : *contexts) {
      population::PopUnit* pop = pop_context.first;
      ProductionContext& context = pop_context.second;

      for (auto& field_info : context.fields) {
        auto* field = field_info.first;
        auto& info = field_info.second;
        if (progressed.count(field) != 0) {
          continue;
        }
        industry::CalculateProductionScale(pop->wealth(), &context, field);
        info.evaluator->SelectCandidate(&context, field);

        const auto& decision = info.decision;
        if (!decision.has_selected()) {
          continue;
        }

        const auto& selected = decision.selected();
        const industry::Production* chain =
            context.production_map->at(selected.name());
        market::Measure max_scale_u = 0;
        if (field->has_progress()) {
          max_scale_u = field->progress().scaling_u();
        } else {
          max_scale_u = chain->MaxScaleU();
        }

        int var_idx = selected.step_info(0).best_variant();
        auto scale_loss_u =
            max_scale_u + attempts[field] -
            selected.step_info(0).variant(var_idx).possible_scale_u();
        if (scale_loss_u < lowest_scale_loss_u) {
          best_field = field;
          best_pop = pop;
          lowest_scale_loss_u = scale_loss_u;
        }
      }
    }
    if (best_field == NULL) {
      break;
    }

    // If this fails we can retry, but at a lower priority.
    attempts[best_field]++;

    ProductionContext& context = contexts->at(best_pop);
    const auto& decision = context.fields[best_field].decision;
    const auto& selected = decision.selected();
    const industry::Production* chain =
        context.production_map->at(selected.name());

    market::proto::Container used_capital;
    if (selected.step_info_size() < 1) {
      // TODO: This is an error, handle it better.
      continue;
    }

    if (!best_field->has_progress() ||
        best_field->progress().name() != chain->get_name()) {
      *best_field->mutable_progress() = chain->MakeProgress(chain->MaxScaleU());
    }

    int var_idx = selected.step_info(0).best_variant();
    if (!industry::InstallFixedCapital(
            chain->get_step(best_field->progress().step()).variants(var_idx),
            selected.step_info(0).variant(var_idx).possible_scale_u(),
            best_pop->mutable_wealth(), best_field->mutable_fixed_capital(),
            context.market)) {
      continue;
    }

    if (industry::TryProductionStep(
            *chain, selected.step_info(0), best_field,
            best_field->mutable_progress(), best_pop->mutable_wealth(),
            best_pop->mutable_wealth(), &used_capital, context.market)) {
      progressed.emplace(best_field);
      if (!best_field->has_progress()) {
        best_pop->SellSurplus(context.market);
      }
    }
    best_pop->ReturnCapital(&used_capital);
  }
}

}  // namespace

GameWorld::~GameWorld() {
  for (auto& production : production_map_) {
    delete production.second;
  }
  production_map_.clear();
}

// TODO: This really needs to handle errors, e.g. in registering templates.
GameWorld::Scenario::Scenario(proto::Scenario* scenario) {
  proto_.Swap(scenario);
  for (const auto& good : proto_.trade_goods()) {
    market::CreateTradeGood(good);
  }
  auto_production_.insert(auto_production_.end(),
                          proto_.auto_production().pointer_begin(),
                          proto_.auto_production().pointer_end());
  production_chains_.insert(production_chains_.end(),
                            proto_.production_chains().pointer_begin(),
                            proto_.production_chains().pointer_end());
  for (auto& decay_rate : *proto_.mutable_decay_rates()->mutable_quantities()) {
    decay_rate.second = micro::kOneInU - decay_rate.second;
  }

  for (const auto& level : proto_.consumption()) {
    if (market::GetAmount(level.tags(), keywords::kSubsistenceTag) > 0) {
      subsistence_.push_back(&level);
    }
  }

  for (const auto& temp : proto_.unit_templates()) {
    units::Unit::RegisterTemplate(temp);
  }
}

GameWorld::GameWorld(const proto::GameWorld& world, proto::Scenario* scenario)
    : scenario_(scenario),
      default_evaluator_(new industry::decisions::LocalProfitMaximiser()) {

  for (const auto& pop : world.pops()) {
    pops_.emplace_back(new population::PopUnit(pop));
  }

  for (const auto& area : world.areas()) {
    areas_.emplace_back(geography::Area::FromProto(area));
  }

  for (const auto& conn : world.connections()) {
    connections_.emplace_back(geography::Connection::FromProto(conn));
  }

  for (const auto* prod_proto : scenario_.production_chains_) {
    production_map_.emplace(prod_proto->name(), new industry::Production(*prod_proto));
    chain_names_.push_back(prod_proto->name());
  }

  for (const auto& unit : world.units()) {
    units_.emplace_back(units::Unit::FromProto(unit));
  }

  for (const auto& faction : world.factions()) {
    factions_.emplace_back(factions::FactionController::FromProto(faction));
  }
}

void GameWorld::TimeStep(
    industry::decisions::FieldMap<
        industry::decisions::proto::ProductionDecision>* decisions) {
  static PossibilityFilter possible;

  for (auto& area: areas_) {
    auto* market = area->mutable_market();
    for (const auto pop_id : area->Proto()->pop_ids()) {
      auto* pop = population::PopUnit::GetPopId(pop_id);
      if (pop == nullptr) {
        continue;
      }
      pop->StartTurn(scenario_.subsistence_, market);
      pop->AutoProduce(scenario_.auto_production_, market);
    }

    std::unordered_map<population::PopUnit*, ProductionContext> contexts;
    for (auto& field : *area->Proto()->mutable_fields()) {
      auto* pop = population::PopUnit::GetPopId(field.owner_id());
      if (pop == nullptr) {
        continue;
      }
      if (contexts.find(pop) == contexts.end()) {
        contexts[pop].production_map = &production_map_;
        contexts[pop].market = market;
        contexts[pop].fields[&field] = industry::decisions::FieldInfo();
      }
      if (production_evaluators_.find(&field) != production_evaluators_.end()) {
        contexts[pop].fields[&field].evaluator = production_evaluators_[&field];
      } else {
        contexts[pop].fields[&field].evaluator = default_evaluator_;
      }

      for(const auto& chain : production_map_) {
        const industry::Production& prod = *chain.second;
        if (!possible.Filter(field, prod)) {
          continue;
        }

        auto& field_info = contexts[pop].fields[&field];
        field_info.candidates.emplace_back(
            std::make_unique<industry::decisions::proto::ProductionInfo>());
        auto* info = field_info.candidates.back().get();
        info->set_name(chain.first);
        industry::CalculateProductionCosts(prod, *market, field, info);
      }
    }

    RunAreaIndustry(&contexts);

    // Copy decisions into output map.
    for (auto& field : *area->Proto()->mutable_fields()) {
      auto* pop = population::PopUnit::GetPopId(field.owner_id());
      if (pop == nullptr) {
        continue;
      }
      decisions->emplace(&field, contexts[pop].fields[&field].decision);
    }
  }

  // Units all plan simultaneously.
  for (auto& unit : units_) {
    actions::proto::Strategy* strategy = unit->mutable_strategy();
    if (strategy->strategy_case() ==
        actions::proto::Strategy::STRATEGY_NOT_SET) {
      // TODO: Strategic AI.
      continue;
    }
    actions::proto::Plan* plan = unit->mutable_plan();
    if (plan->steps_size() == 0) {
      *plan = ai::MakePlan(*unit, unit->strategy());
    }
  }

  // Execute in single steps.
  while (true) {
    int count = 0;
    for (auto& unit : units_) {
      if (ai::ExecuteStep(unit->plan(), unit.get())) {
        ai::DeleteStep(unit->mutable_plan());
        ++count;
      }
    }
    if (count == 0) {
      break;
    }
  }

  // Must consume with levels in the outside loop, otherwise
  // one POP may eat everything and leave nothing for others.
  // Need to do by areas to get the markets.
  for (auto& area: areas_) {
    for (const auto& level : scenario_.proto_.consumption()) {
      for (const auto pop_id : area->Proto()->pop_ids()) {
        auto* pop = population::PopUnit::GetPopId(pop_id);
        if (pop == nullptr) {
          continue;
        }
        pop->Consume(level, area->mutable_market());
      }
    }
  }

  for (auto& pop : pops_) {
    pop->EndTurn(scenario_.proto_.decay_rates());
  }
  for (auto& area: areas_) {
    market::proto::Container volumes =
        area->mutable_market()->Proto()->volume();
    area->mutable_market()->FindPrices();
    area->mutable_market()->DecayGoods(scenario_.proto_.decay_rates());
    for (auto& field : *area->Proto()->mutable_fields()) {
      micro::MultiplyU(*field.mutable_fixed_capital(),
                       scenario_.proto_.decay_rates());
    }
  }
}

void GameWorld::SaveToProto(proto::GameWorld* proto) const {
  for (const auto& pop: pops_) {
    *proto->add_pops() = *pop->Proto();
  }
  for (const auto& area : areas_) {
    auto* area_proto = proto->add_areas();
    *area_proto = *area->Proto();
    *area_proto->mutable_market() = *area->mutable_market()->Proto();
  }
  for (const auto& conn : connections_) {
    auto* conn_proto = proto->add_connections();
    *conn_proto = conn->Proto();
  }

  for (const auto& unit : units_) {
    market::CleanContainer(unit->mutable_resources());
    auto& unit_proto = *proto->add_units();
    unit_proto = unit->Proto();
  }

  for (const auto& faction : factions_) {
    auto* faction_proto = proto->add_factions();
    *faction_proto = faction->Proto();
  }
}

void GameWorld::SetProductionEvaluator(
    uint64 area_id, uint64 field_idx,
    industry::decisions::ProductionEvaluator* eval) {
  geography::Area* area = geography::Area::GetById(area_id);
  if (area == NULL) {
    // TODO: Maybe a better handling?
    return;
  }

  if (field_idx >= area->num_fields()) {
    // TODO: Error handling!
    return;
  }
  auto* field = area->mutable_field(field_idx);
  if (eval == NULL) {
    production_evaluators_.erase(field);
  } else {
    production_evaluators_[field] = eval;
  }
}

} // namespace game
