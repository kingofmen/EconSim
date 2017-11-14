#include "game/game_world.h"

#include <iostream>

#include "industry/decisions/production_evaluator.h"

using geography::proto::Field;

namespace game {

GameWorld::Scenario::Scenario(proto::Scenario* scenario) {
  proto_.Swap(scenario);
  auto_production_.insert(auto_production_.end(),
                          proto_.auto_production().pointer_begin(),
                          proto_.auto_production().pointer_end());
  production_chains_.insert(production_chains_.end(),
                            proto_.production_chains().pointer_begin(),
                            proto_.production_chains().pointer_end());
  for (auto& decay_rate : *proto_.mutable_decay_rates()->mutable_quantities()) {
    decay_rate.second = 1 - decay_rate.second;
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
      pop->AutoProduce(scenario_.auto_production_, area->GetPrices());
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
        pop_field.first->Consume(level,
                                 area->mutable_market()->Proto()->prices());
      }
    }
  }

  for (auto& pop : pops_) {
    pop->EndTurn(scenario_.proto_.decay_rates());
  }
}

void GameWorld::SaveToProto(proto::GameWorld* proto) const {

  for (const auto& pop: pops_) {
    *proto->add_pops() = *pop->Proto();
  }
  for (const auto& area: areas_) {
    *proto->add_areas() = *area->Proto();
  }
}

} // namespace game
