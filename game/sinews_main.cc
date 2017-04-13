#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "game/proto/game_world.pb.h"
#include "game/game_world.h"
#include "google/protobuf/stubs/logging.h"
#include "google/protobuf/text_format.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "util/status/status.h"

util::Status ParseProtoFile(const std::string& filename,
                            google::protobuf::Message* proto) {
  std::ifstream reader(filename);
  if (!reader.good()) {
    return util::InvalidArgumentError("Could not open file");
  }
  google::protobuf::io::IstreamInputStream input(&reader);
  if (!google::protobuf::TextFormat::Parse(&input, proto)) {
    return util::InvalidArgumentError("Error parsing file");
  }

  reader.close();
  return util::OkStatus();
}

int main(int /*argc*/, char** /*argv*/) {
  game::proto::GameWorld world_proto;
  auto status = ParseProtoFile(".\\test_data\\test.pb.txt", &world_proto);
  if (!status.ok()) {
    std::cout << status.error_message() << "\n";
    return 1;
  }

  game::proto::Scenario scenario;
  status = ParseProtoFile(".\\test_data\\test_scenario.pb.txt", &scenario);
  if (!status.ok()) {
    std::cout << status.error_message() << "\n";
    return 1;
  }

  game::GameWorld game_world(world_proto, &scenario);

  game_world.TimeStep();

  world_proto.Clear();
  game_world.SaveToProto(&world_proto);
  std::cout << world_proto.DebugString() << "\n";

  return 0;
}

