#include "game/validation/validation.h"

#include <iostream>
#include <unordered_set>

#include "absl/strings/str_join.h"
#include "game/proto/game_world.pb.h"
#include "gtest/gtest.h"
#include "util/proto/file.h"

namespace game {
namespace validation {
namespace {

google::protobuf::util::Status ReadFile(const std::string filename,
                                        google::protobuf::Message* proto) {
  // This is a workaround for Bazel issues 4102 and 4292. When they are
  // fixed, use TEST_SRCDIR/TEST_WORKSPACE instead.
  const std::string kTestDir = "C:/Users/Rolf/base";
  const std::string kTestDataLocation = "game/validation/test_data";
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
    "Auto production: Good nonesuch does not exist.",
    "unobtainium has bad decay rate -1",
    "handwavium has bad bulk 0",
    "phlebotinum has bad weight 0",
    "Production bad_chain outputs: Good output does not exist.",
    "Production bad_chain step 1 variant 1 consumables: Good labor does not exist.",
    "Production bad_chain step 1 variant 1 movable capital: Good handwaving does not exist.",
    "Production bad_chain step 1 variant 1 fixed capital: Good captal does not exist.",
    "Production bad_chain step 1 variant 1 raw materials: Good unobtanum does not exist.",
    "Production bad_chain step 1 variant 1 install cost: Good phlebotinium does not exist.",
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
}  // namespace game
