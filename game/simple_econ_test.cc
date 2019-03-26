// Test of a simple self-sustaining economy.

#include <cstdlib>
#include <fstream>

#include "absl/strings/str_join.h"
#include "absl/strings/substitute.h"
#include "game/game_world.h"
#include "game/proto/game_world.pb.h"
#include "geography/proto/geography.pb.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/stubs/logging.h"
#include "google/protobuf/text_format.h"
#include "gtest/gtest.h"
#include "industry/proto/decisions.pb.h"
#include "market/goods_utils.h"
#include "util/proto/file.h"
#include "util/status/status.h"

namespace simple_economy_test {
const std::string kTestDataLocation = "game/test_data";
const std::string kSimpleSetup = "simple.pb.txt";
const std::string kSimpleEconomy = "simple_economy.pb.txt";

const std::string kFixcapSetup = "fixcap.pb.txt";
const std::string kFixcapEconomy = "fixcap_economy.pb.txt";

namespace {

void ReadFile(const std::string filename, google::protobuf::Message* proto) {
  // This is a workaround for Bazel issues 4102 and 4292. When they are
  // fixed, use TEST_SRCDIR/TEST_WORKSPACE instead.
  const std::string kTestDir = "C:/Users/Rolf/base";
  auto status = util::proto::ParseProtoFile(
      absl::StrJoin({kTestDir, kTestDataLocation, filename}, "/"), proto);
  ASSERT_TRUE(status.ok()) << status.error_message();
}

}

class SimpleEconomyTest : public testing::Test {
protected:
  game::proto::GameWorld world_proto_;
  game::proto::Scenario scenario_;
};

TEST_F(SimpleEconomyTest, TestStablePrices) {
  ReadFile(kSimpleSetup, &world_proto_);
  ReadFile(kSimpleEconomy, &scenario_);
  game::GameWorld game_world(world_proto_, &scenario_);
  auto initial_prices = world_proto_.areas(0).market().prices_u();
  std::unordered_map<geography::proto::Field*,
                     industry::decisions::proto::ProductionDecision>
      production_info;
  for (int i = 0; i < 10; ++i) {
    production_info.clear();
    game_world.TimeStep(&production_info);
  }
  world_proto_.Clear();
  game_world.SaveToProto(&world_proto_);
  auto final_prices = world_proto_.areas(0).market().prices_u();
  for (const auto& good : initial_prices.quantities()) {
    EXPECT_NE(good.second, 0);
    EXPECT_EQ(good.second, market::GetAmount(final_prices, good.first));
  }
  for (const auto& good : final_prices.quantities()) {
    EXPECT_EQ(good.second, market::GetAmount(initial_prices, good.first));
  }
}

} // namespace simple_economy_test
