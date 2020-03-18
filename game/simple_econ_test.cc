// Test of a simple self-sustaining economy.

#include <cstdlib>
#include <fstream>

#include "absl/strings/str_join.h"
#include "absl/strings/substitute.h"
#include "game/game_world.h"
#include "games/setup/proto/setup.pb.h"
#include "games/setup/setup.h"
#include "games/setup/validation/validation.h"
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
const std::string kWorld = "world.pb.txt";
const std::string kAutoProd = "auto_production.pb.txt";
const std::string kChains = "chains.pb.txt";
const std::string kTradeGoods = "goods.pb.txt";
const std::string kConsumption = "consumption.pb.txt";
const std::string kUnits = "units.pb.txt";


class EconomyTest : public testing::Test {
protected:
  games::setup::proto::GameWorld world_proto_;
  games::setup::proto::Scenario scenario_;

  google::protobuf::util::Status LoadTestData(const std::string& location) {
    market::ClearGoods();
    games::setup::proto::ScenarioFiles config;
    const std::string kTestDir = std::getenv("TEST_SRCDIR");
    const std::string kWorkdir = std::getenv("TEST_WORKSPACE");
    const std::string kBase =
        absl::StrJoin({kTestDir, kWorkdir, kTestDataLocation, location}, "/");

    config.add_auto_production(absl::StrJoin({kBase, kAutoProd}, "/"));
    config.add_production_chains(absl::StrJoin({kBase, kChains}, "/"));
    config.add_trade_goods(absl::StrJoin({kBase, kTradeGoods}, "/"));
    config.add_consumption(absl::StrJoin({kBase, kConsumption}, "/"));
    config.add_unit_templates(absl::StrJoin({kBase, kUnits}, "/"));
    auto status = games::setup::LoadScenario(config, &scenario_);
    if (!status.ok()) return status;

    config.set_world_file(absl::StrJoin({kBase, kWorld}, "/"));
    return games::setup::LoadWorld(config, &world_proto_);
  }

  void validate() {
    std::vector<std::string> errors =
        games::setup::validation::Validate(scenario_, world_proto_);
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
  auto status = LoadTestData("simple");
  EXPECT_OK(status) << status.error_message();
  status = SteadyStateTest();
  EXPECT_TRUE(status.ok()) << status.error_message() << "\n"
                           << world_proto_.DebugString();
}

TEST_F(EconomyTest, TestFixcapSteadyState) {
  auto status = LoadTestData("fixcap");
  EXPECT_OK(status) << status.error_message();
  status = SteadyStateTest();
  EXPECT_TRUE(status.ok()) << status.error_message() << "\n"
                           << world_proto_.DebugString();
}

TEST_F(EconomyTest, TestTradingSteadyState) {
  auto status = LoadTestData("trade");
  EXPECT_OK(status) << status.error_message();
  status = SteadyStateTest();
  EXPECT_TRUE(status.ok()) << status.error_message() << "\n"
                           << world_proto_.DebugString();
}

} // namespace simple_economy_test
