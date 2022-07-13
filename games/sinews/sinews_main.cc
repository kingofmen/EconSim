#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "absl/strings/str_join.h"
#include "games/setup/setup.h"
#include "games/setup/proto/setup.pb.h"
#include "games/sinews/game_world.h"
#include "games/geography/proto/geography.pb.h"
#include "games/industry/proto/decisions.pb.h"
#include "util/proto/file.h"
#include "util/status/status.h"

int main(int /*argc*/, char** /*argv*/) {
  games::setup::proto::Scenario scenario;
  games::setup::proto::GameWorld world_proto;
  games::setup::proto::ScenarioFiles config;
  const std::string kBase = ".\\test_data\\simple";
  const std::string kWorld = "world.pb.txt";
  const std::string kAutoProd = "auto_production.pb.txt";
  const std::string kChains = "chains.pb.txt";
  const std::string kTradeGoods = "goods.pb.txt";
  const std::string kConsumption = "consumption.pb.txt";
  const std::string kUnits = "units.pb.txt";

  config.add_auto_production(absl::StrJoin({kBase, kAutoProd}, "/"));
  config.add_production_chains(absl::StrJoin({kBase, kChains}, "/"));
  config.add_trade_goods(absl::StrJoin({kBase, kTradeGoods}, "/"));
  config.add_consumption(absl::StrJoin({kBase, kConsumption}, "/"));
  config.add_unit_templates(absl::StrJoin({kBase, kUnits}, "/"));
  config.set_world_file(absl::StrJoin({kBase, kWorld}, "/"));

  auto status = games::setup::LoadScenario(config, &scenario);
  if (!status.ok()) {
    std::cout << status.message() << "\n";
    return 1;
  }

  status = games::setup::LoadWorld(config, &world_proto);
  if (!status.ok()) {
    std::cout << status.message() << "\n";
    return 1;
  }

  game::GameWorld game_world(world_proto, scenario);
  std::unordered_map<geography::proto::Field*,
                     industry::decisions::proto::ProductionDecision>
      production_info;
  for (int i = 0; i < 10; ++i) {
    std::cout << "Turn " << i << " begins\n";
    production_info.clear();
    game_world.TimeStep(&production_info);
    for (const auto& info : production_info) {
      std::cout << "\nField:\n"
                << info.first->DebugString() << "decision:\n"
                << info.second.DebugString();
    }
  }

  world_proto.Clear();
  game_world.SaveToProto(&world_proto);
  std::cout << world_proto.DebugString() << "\n";

  return 0;
}

