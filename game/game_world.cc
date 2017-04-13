#include "game/game_world.h"

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
    : scenario_(scenario) {

  for (const auto& pop : world.pops()) {
    pops_.emplace_back(new population::PopUnit(pop));
  }

  for (const auto& area : world.areas()) {
    areas_.emplace_back(new geography::Area(area));
  }
}

void GameWorld::TimeStep() {
  for (auto& area: areas_) {
    for (const auto pop_id : area->pop_ids()) {
      auto* pop = population::PopUnit::GetPopId(pop_id);
      pop->AutoProduce(scenario_.auto_production_, area->GetPrices());
    }
  }

  for (auto& pop : pops_) {
    pop->DecayWealth(scenario_.proto_.decay_rates());
  }
}

void GameWorld::SaveToProto(proto::GameWorld* proto) const {
  for (const auto& pop: pops_) {
    *proto->add_pops() = *pop;
  }
  for (const auto& area: areas_) {
    *proto->add_areas() = *area;
  }
}

} // namespace game
