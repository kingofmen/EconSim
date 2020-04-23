#include "games/setup/validation/validation.h"

#include <iostream>
#include <unordered_set>

#include "absl/strings/str_join.h"
#include "games/setup/proto/setup.pb.h"
#include "gtest/gtest.h"
#include "util/proto/file.h"

namespace games {
namespace setup {
namespace validation {
namespace {

const std::string kTestDataLocation = "games/setup/validation/test_data";

google::protobuf::util::Status ReadFile(const std::string filename,
                                        google::protobuf::Message* proto) {
  const std::string kTestDir = std::getenv("TEST_SRCDIR");
  const std::string kWorkdir = std::getenv("TEST_WORKSPACE");
  return util::proto::ParseProtoFile(
      absl::StrJoin({kTestDir, kWorkdir, kTestDataLocation, filename}, "/"),
      proto);
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

  games::setup::proto::GameWorld world_proto_;
  games::setup::proto::Scenario scenario_;
};

TEST_F(ValidationTest, TestAllValidations) {
  Validator extra = [](const games::setup::proto::GameWorld& p) {
    return std::vector<std::string>{"extra error"};
  };
  RegisterValidator("outside", extra);
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
    "Area 0 field 0:: Good stable_dark_matter does not exist.",
    "Area 0 field 0:: Good testonium does not exist.",
    "Area ID number: 2\n is not unique",
    "Bad area ID: number: 0\n",
    "Pop without ID: \"\"",
    "Pop ID 1 is not unique",
    "Pop unit 1: Good gold does not exist.",
    "Connection 2 does not connect",
    "Connection 2 has bad length 0",
    "Connection 2 has bad width 0",
    "Connection 3 has A end number: 5\n, which doesn't exist",
    "Connection 3 has Z end number: 6\n, which doesn't exist",
    "Connection ID 3 is not unique",
    "Unit {Bad unit, 1} has bad kind",
    "Unit {Generic unit, 4} has no location",
    "Unit {Generic unit, 1}: Good iron does not exist.",
    "Bad unit ID: unit_id {\n  kind: \"Generic unit\"\n}\n",
    "Unit without ID: \"\"",
    "Unit {Generic unit, 1} has nonexistent location number: 5\n",
    "Unit {Generic unit, 2} has nonexistent connection 12",
    "Unit {Generic unit, 3} is in connection 3 which does not connect source number: 1\n",
    "Unit {Generic unit, 3} is not unique",
    "outside : extra error",
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
