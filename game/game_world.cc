#include "game/game_world.h"

#include <iostream>

#include "industry/decisions/production_evaluator.h"
#include "industry/worker.h"
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
  static industry::decisions::LocalProfitMaximiser evaluator;

  std::unordered_set<Field*> progressed;
  bool progress = true;
  while (progress) {
    progress = false;
    for (auto& pop_context : *contexts) {
      population::PopUnit* pop = pop_context.first;
      ProductionContext& context = pop_context.second;

      for (auto* field : context.fields) {
        if (progressed.count(field) != 0) {
          continue;
        }
        industry::CalculateProductionScale(pop->wealth(), &context, field);
        industry::SelectProduction(evaluator, &context, field);

        auto& decision = context.decisions->at(field);
        if (!decision.has_selected()) {
          continue;
        }

        auto& selected = decision.selected();
        auto* chain = context.production_map->at(selected.name());
        if (!field->has_progress()) {
          *field->mutable_progress() =
              chain->MakeProgress(selected.max_scale_u());
        }

        market::proto::Container used_capital;
        if (selected.step_info_size() < 1) {
          // TODO: This is an error, handle it better.
          continue;
        }

        if (industry::TryProductionStep(
                *chain, selected.step_info(0), field, field->mutable_progress(),
                pop->mutable_wealth(), pop->mutable_wealth(), &used_capital,
                context.market)) {
          progress = true;
          progressed.emplace(field);
          if (!field->has_progress()) {
            pop->SellSurplus(context.market);
          }
        }
        pop->ReturnCapital(&used_capital);
      }
    }
  }
}

}  // namespace

GameWorld::~GameWorld() {
  for (auto& production : production_map_) {
    delete production.second;
  }
  production_map_.clear();
}


GameWorld::Scenario::Scenario(proto::Scenario* scenario) {
  proto_.Swap(scenario);
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
}

GameWorld::GameWorld(const proto::GameWorld& world, proto::Scenario* scenario)
    : scenario_(scenario), local_profit_maximiser_() {

  for (const auto& pop : world.pops()) {
    pops_.emplace_back(new population::PopUnit(pop));
  }

  for (const auto& area : world.areas()) {
    areas_.emplace_back(new geography::Area(area));
  }

  for (const auto* prod_proto : scenario_.production_chains_) {
    production_map_.emplace(prod_proto->name(), new industry::Production(*prod_proto));
  }
}

void GameWorld::TimeStep(industry::decisions::DecisionMap* decisions) {
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

    std::unordered_set<Field*> fields;
    std::unordered_map<population::PopUnit*, ProductionContext> contexts;
    for (auto& field : *area->Proto()->mutable_fields()) {
      auto* pop = population::PopUnit::GetPopId(field.owner_id());
      if (pop == nullptr) {
        continue;
      }
      if (contexts.find(pop) == contexts.end()) {
        contexts[pop].production_map = &production_map_;
        contexts[pop].market = market;
        contexts[pop].decisions = decisions;
      }
      contexts[pop].fields.emplace(&field);
      fields.emplace(&field);
      decisions->emplace(&field,
                         industry::decisions::proto::ProductionDecision());
      for(const auto& chain : production_map_) {
        const industry::Production& prod = *chain.second;
        if (!possible.Filter(field, prod)) {
          continue;
        }

        contexts[pop].candidates[&field].emplace_back();
        industry::decisions::proto::ProductionInfo* info =
            &(contexts[pop].candidates[&field].back());
        info->set_name(chain.first);
        industry::CalculateProductionCosts(prod, *market, field, info);
      }
    }

    RunAreaIndustry(&contexts);

    for (const auto& level : scenario_.proto_.consumption()) {
      for (const auto pop_id : area->Proto()->pop_ids()) {
        auto* pop = population::PopUnit::GetPopId(pop_id);
        if (pop == nullptr) {
          continue;
        }
        pop->Consume(level, market);
      }
    }
  }

  for (auto& pop : pops_) {
    pop->EndTurn(scenario_.proto_.decay_rates());
    std::cout << pop->Proto()->DebugString();
  }
  for (auto& area: areas_) {
    market::proto::Container volumes =
        area->mutable_market()->Proto()->volume();
    area->mutable_market()->FindPrices();
    area->mutable_market()->DecayGoods(scenario_.proto_.decay_rates());
    PrintMarket(*area->mutable_market()->Proto(), volumes);
  }
}

void GameWorld::SaveToProto(proto::GameWorld* proto) const {
  for (const auto& pop: pops_) {
    *proto->add_pops() = *pop->Proto();
  }
  for (const auto& area: areas_) {
    auto* area_proto = proto->add_areas();
    *area_proto = *area->Proto();
    *area_proto->mutable_market() = *area->mutable_market()->Proto();
  }
}

} // namespace game
