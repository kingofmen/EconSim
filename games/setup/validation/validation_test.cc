#include "games/setup/validation/validation.h"

#include <iostream>
#include <unordered_set>

#include "absl/strings/str_join.h"
#include "game/proto/game_world.pb.h"
#include "gtest/gtest.h"
#include "util/proto/file.h"

namespace games {
namespace setup {
namespace validation {
namespace {

google::protobuf::util::Status ReadFile(const std::string filename,
                                        google::protobuf::Message* proto) {
  // This is a workaround for Bazel issues 4102 and 4292. When they are
  // fixed, use TEST_SRCDIR/TEST_WORKSPACE instead.
  const std::string kTestDir = "C:/Users/Rolf/base";
  const std::string kTestDataLocation = "games/setup/validation/test_data";
  return util::proto::ParseProtoFile(
      absl::StrJoin({kTestDir, kTestDataLocation, filename}, "/"), proto);
}

}

class ValidationTest : public testing::Test {
protected:
  void SetUp() override {
    auto status = ReadFile("bad_scenario.pb.txt", &scenario_);
    if (!status.ok()) {
      std::cout << status.error_message() << "\n";
    }
    status = ReadFile("bad_world.pb.txt", &world_proto_);
    if (!status.ok()) {
      std::cout << status.error_message() << "\n";
    }
  }

  game::proto::GameWorld world_proto_;
  game::proto::Scenario scenario_;
};

TEST_F(ValidationTest, TestAllValidations) {
  std::unordered_set<std::string> expected = {
    // Scenario errors:
    "Auto production: Good nonesuch does not exist.",
    "unobtainium has bad decay rate -1",
    "handwavium has no bulk",
    "phlebotinum has no weight",
    "Production bad_chain outputs: Good output does not exist.",
    "Production bad_chain step 1 variant 1 consumables: Good labor does not exist.",
    "Production bad_chain step 1 variant 1 movable capital: Good handwaving does not exist.",
    "Production bad_chain step 1 variant 1 fixed capital: Good captal does not exist.",
    "Production bad_chain step 1 variant 1 raw materials: Good unobtanum does not exist.",
    "Production bad_chain step 1 variant 1 install cost: Good phlebotinium does not exist.",
    "Consumption bad_level package 1 consumption: Good phlebotinium does not exist.",
    "Consumption bad_level package 1 capital: Good handwaves does not exist.",
    // World errors:
    "Area without ID: \"\"",
    "Area ID 2 is not unique",
    "Bad area ID: 0",
    "Pop without ID: \"\"",
    "Pop ID 1 is not unique",
    "Pop unit 1: Good gold does not exist.",
    "Connection 2 does not connect",
    "Connection 2 has bad length 0",
    "Connection 2 has bad width 0",
    "Connection 3 has A end 5, which doesn't exist",
    "Connection 3 has Z end 6, which doesn't exist",
    "Connection ID 3 is not unique",
    "Unit {10, 1} has bad type",
    "Unit {10, 1} has no location",
    "Unit {1, 1}: Good iron does not exist.",
    "Bad unit ID: unit_id {\n  type: 1\n}\n",
    "Unit without ID: \"\"",
    "Unit {1, 1} has nonexistent location 5",
    "Unit {1, 2} has nonexistent connection 12",
    "Unit {1, 3} is in connection 3 which does not connect source 1",
    "Unit {1, 3} is not unique",
  };
  auto errors = Validate(scenario_, world_proto_);
  for (const auto& error : errors) {
    EXPECT_TRUE(expected.find(error) != expected.end())
        << "Unexpected error: " << error;
    expected.erase(error);
  }

  EXPECT_EQ(0, expected.size());
  for (const auto& err : expected) {
    std::cout << "Expected error did not appear: " << err << "\n";
  }
}

}  // namespace validation
}  // namespace setup
}  // namespace games
