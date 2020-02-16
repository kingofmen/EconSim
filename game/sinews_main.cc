#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "games/setup/proto/setup.pb.h"
#include "game/game_world.h"
#include "geography/proto/geography.pb.h"
#include "google/protobuf/stubs/logging.h"
#include "google/protobuf/text_format.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "industry/proto/decisions.pb.h"
#include "util/proto/file.h"

int main(int /*argc*/, char** /*argv*/) {
  games::setup::proto::GameWorld world_proto;
  auto status =
      util::proto::ParseProtoFile(".\\test_data\\simple.pb.txt", &world_proto);
  if (!status.ok()) {
    std::cout << status.error_message() << "\n";
    return 1;
  }

  games::setup::proto::Scenario scenario;
  status = util::proto::ParseProtoFile(".\\test_data\\simple_economy.pb.txt",
                                       &scenario);
  if (!status.ok()) {
    std::cout << status.error_message() << "\n";
    return 1;
  }

  game::GameWorld game_world(world_proto, &scenario);
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

