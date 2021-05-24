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
  games::setup::proto::GameWorld world_proto_;
  games::setup::proto::Scenario scenario_;
};

TEST_F(ValidationTest, TestAllValidations) {
  auto status = ReadFile("bad_scenario.pb.txt", &scenario_);
  EXPECT_TRUE(status.ok()) << status.error_message();
  status = ReadFile("bad_world.pb.txt", &world_proto_);
  EXPECT_TRUE(status.ok()) << status.error_message();

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
    "(connection, 2) does not connect",
    "(connection, 2) has bad length 0",
    "(connection, 2) has bad width 0",
    "(connection, 3) has A end (, 5), which doesn't exist",
    "(connection, 3) has Z end (, 6), which doesn't exist",
    "Connection ID (connection, 3) is not unique",
    "Unit {Bad unit, 1} has bad kind",
    "Unit {Generic unit, 4} has no location",
    "Unit {Generic unit, 1}: Good iron does not exist.",
    "Bad unit ID: unit_id {\n  kind: \"Generic unit\"\n}\n",
    "Unit without ID: \"\"",
    "(Generic unit, 1) has nonexistent location (, 5)",
    "Unit (Generic unit, 2) has nonexistent connection (connection, 12)",
    "(Generic unit, 3) is in (connection, 3) which does not connect source (, 1)",
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

TEST_F(ValidationTest, TestUnitFactions) {
  world_proto_.Clear();
  auto status = ReadFile("unit_factions.pb.txt", &world_proto_);
  EXPECT_TRUE(status.ok()) << status.error_message();

  auto errors = optional::UnitFactions(world_proto_);
  std::unordered_set<std::string> expected = {
    "Faction without faction_id: ",
    "Duplicate faction ID (faction, 1)",
    "Unit (unit, 1) has no faction ID",
    "Unit (unit, 2) belongs to unknown faction (faction, 101)",
  };
  for (const auto& error : errors) {
    EXPECT_TRUE(expected.find(error) != expected.end())
        << "Unexpected error: \"" << error << "\"";
    expected.erase(error);
  }

  EXPECT_EQ(0, expected.size());
  for (const auto& err : expected) {
    std::cout << "Expected error did not appear: \"" << err << "\"\n";
  }  
}

}  // namespace validation
}  // namespace setup
}  // namespace games
