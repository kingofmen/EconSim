#include <fstream>
#include <iostream>

#include "game/proto/game_world.pb.h"
#include "geography/proto/geography.pb.h"
#include "industry/proto/industry.pb.h"

#include "google/protobuf/stubs/logging.h"
#include "google/protobuf/text_format.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>

int main(int /*argc*/, char** /*argv*/) {
  std::ifstream game_reader("test_data\\test.pb.txt");
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

  std::cout << game_world.DebugString() << "\n";
  
  return 0;
}

