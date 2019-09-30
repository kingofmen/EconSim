// Test of a simple self-sustaining economy.

#include <cstdlib>
#include <fstream>

#include "absl/strings/str_join.h"
#include "absl/strings/substitute.h"
#include "game/game_world.h"
#include "game/proto/game_world.pb.h"
#include "game/validation/validation.h"
#include "geography/proto/geography.pb.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/stubs/logging.h"
#include "google/protobuf/stubs/status.h"
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

const std::string kTradeSetup = "trade.pb.txt";
const std::string kTradeEconomy = "trade_economy.pb.txt";

namespace {

google::protobuf::util::Status ReadFile(const std::string filename,
                                        google::protobuf::Message* proto) {
  // This is a workaround for Bazel issues 4102 and 4292. When they are
  // fixed, use TEST_SRCDIR/TEST_WORKSPACE instead.
  const std::string kTestDir = "C:/Users/Rolf/base";
  return util::proto::ParseProtoFile(
      absl::StrJoin({kTestDir, kTestDataLocation, filename}, "/"), proto);
}

}

class EconomyTest : public testing::Test {
protected:
  game::proto::GameWorld world_proto_;
  game::proto::Scenario scenario_;

  google::protobuf::util::Status ReadWorld(const std::string& setup,
                                           const std::string& scenario) {
    market::ClearGoods();
    auto status = ReadFile(setup, &world_proto_);
    if (!status.ok()) return status;
    status = ReadFile(scenario, &scenario_);
    if (!status.ok()) return status;
    return util::OkStatus();
  }

  void validate() {
    std::vector<std::string> errors =
        game::validation::Validate(scenario_, world_proto_);
    EXPECT_EQ(0, errors.size());
    for (const auto& error : errors) {
      std::cout << "Validation error: " << error << "\n";
    }
  }

  google::protobuf::util::Status SteadyStateTest() {
    validate();
    game::GameWorld game_world(world_proto_, &scenario_);
    std::vector<market::proto::Container> initial_prices;
    for (const auto& area : world_proto_.areas()) {
      initial_prices.push_back(area.market().prices_u());
    }
    std::unordered_map<geography::proto::Field*,
                       industry::decisions::proto::ProductionDecision>
        production_info;
    for (int i = 0; i < 10; ++i) {
      production_info.clear();
      game_world.TimeStep(&production_info);

      world_proto_.Clear();
      game_world.SaveToProto(&world_proto_);

      for (int aa = 0; aa < world_proto_.areas_size(); ++aa) {
        const auto& area = world_proto_.areas(aa);
        auto& current_prices = area.market().prices_u();
        for (const auto& good : initial_prices[aa].quantities()) {
          auto curr_price = market::GetAmount(current_prices, good.first);
          if (good.second != curr_price) {
            return util::FailedPreconditionError(
                absl::Substitute("Turn $0 area $4: $1 price $2 does not match initial $3",
                                 i, good.first, curr_price, good.second, area.id()));
          }
        }
        for (const auto& good : current_prices.quantities()) {
          auto init_price = market::GetAmount(initial_prices[aa], good.first);
          if (good.second != init_price) {
            return util::FailedPreconditionError(absl::Substitute(
                "Turn $0 area $4: $1 initial price $3 does not match current "
                "$2",
                i, good.first, good.second, init_price, area.id()));
          }
        }
      }
    }
    return util::OkStatus();
  }
};

TEST_F(EconomyTest, TestSimpleSteadyState) {
  auto status = ReadWorld(kSimpleSetup, kSimpleEconomy);
  EXPECT_OK(status) << status.error_message();
  status = SteadyStateTest();
  EXPECT_TRUE(status.ok()) << status.error_message() << "\n"
                           << world_proto_.DebugString();
}

TEST_F(EconomyTest, TestFixcapSteadyState) {
  auto status = ReadWorld(kFixcapSetup, kFixcapEconomy);
  EXPECT_OK(status) << status.error_message();
  status = SteadyStateTest();
  EXPECT_TRUE(status.ok()) << status.error_message() << "\n"
                           << world_proto_.DebugString();
}

TEST_F(EconomyTest, TestTradingSteadyState) {
  auto status = ReadWorld(kTradeSetup, kTradeEconomy);
  EXPECT_OK(status) << status.error_message();
  status = SteadyStateTest();
  EXPECT_TRUE(status.ok()) << status.error_message() << "\n"
                           << world_proto_.DebugString();
}

} // namespace simple_economy_test
