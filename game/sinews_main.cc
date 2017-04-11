#include <fstream>
#include <iostream>
#include <memory>

#include "game/proto/game_world.pb.h"
#include "geography/geography.h"
#include "geography/proto/geography.pb.h"
#include "google/protobuf/stubs/logging.h"
#include "google/protobuf/text_format.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "industry/proto/industry.pb.h"
#include "population/popunit.h"

int main(int /*argc*/, char** /*argv*/) {
  std::ifstream game_reader(".\\test_data\\test.pb.txt");
  if (!game_reader.good()) {
    std::cout << "Error opening file\n";
    return 1;
  }

  game::proto::GameWorld game_world;
  google::protobuf::io::IstreamInputStream input(&game_reader);
  if (!google::protobuf::TextFormat::Parse(&input, &game_world)) {
    std::cout << "Parsing error\n";
    return 1;
  }

  game_reader.close();

  std::vector<std::unique_ptr<population::PopUnit>> pops;
  for (const auto& pop: game_world.pops()) {
    pops.emplace_back(new population::PopUnit(pop));
  }

  std::vector<const population::proto::AutoProduction*> auto_production(
      game_world.auto_production().pointer_begin(),
      game_world.auto_production().pointer_end());

  std::vector<std::unique_ptr<geography::Area>> areas;
  for (const auto& area: game_world.areas()) {
    areas.emplace_back(new geography::Area(area));
  }
  
  for (auto& area: areas) {
    for (const auto pop_id : area->pop_ids()) {
      auto* pop = population::PopUnit::GetPopId(pop_id);
      pop->AutoProduce(auto_production, area->GetPrices());
    }
  }
  std::cout << game_world.DebugString() << "\n";

  return 0;
}

