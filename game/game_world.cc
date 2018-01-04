#include "game/game_world.h"

#include <iostream>

#include "industry/decisions/production_evaluator.h"
#include "util/arithmetic/microunits.h"
#include "util/keywords/keywords.h"

using geography::proto::Field;

namespace game {
namespace {

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

}  // namespace


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

void GameWorld::TimeStep(industry::decisions::DecisionMap* production_info) {
  for (auto& area: areas_) {
    for (const auto pop_id : area->Proto()->pop_ids()) {
      auto* pop = population::PopUnit::GetPopId(pop_id);
      if (pop == nullptr) {
        continue;
      }
      pop->StartTurn(scenario_.subsistence_, area->mutable_market());
      pop->AutoProduce(scenario_.auto_production_, area->mutable_market());
    }
    std::unordered_map<population::PopUnit*, std::vector<Field*>> fields;
    for (auto& field : *area->Proto()->mutable_fields()) {
      auto* pop = population::PopUnit::GetPopId(field.owner_id());
      if (pop == nullptr) {
        continue;
      }
      fields[pop].emplace_back(&field);
    }
    
    bool progress = true;
    while (progress) {
      progress = false;
      for (auto& pop_field : fields) {
        industry::decisions::ProductionContext context = {
            production_map_, pop_field.second, area->mutable_market()};
        if (pop_field.first->Produce(context, production_info)) {
          progress = true;
        }
      }
    }
    for (auto& pop_field : fields) {
      for (const auto& level : scenario_.proto_.consumption()) {
        pop_field.first->Consume(level, area->mutable_market());
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
