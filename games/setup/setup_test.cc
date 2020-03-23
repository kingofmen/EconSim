#include "games/setup/setup.h"

#include "absl/strings/str_join.h"
#include "gtest/gtest.h"
#include "src/google/protobuf/util/message_differencer.h"

const std::string kTestDataLocation = "games/setup/test_data";
const std::string kWorld = "world.pb.txt";
const std::string kAutoProd = "auto_production.pb.txt";
const std::string kChains = "chains.pb.txt";
const std::string kTradeGoods = "goods.pb.txt";
const std::string kConsumption = "consumption.pb.txt";
const std::string kUnits = "units.pb.txt";


TEST(SetupTest, TestIdempotency) {
  games::setup::proto::Scenario scenario;
  games::setup::proto::GameWorld gameworld;
  games::setup::proto::ScenarioFiles config;

  const std::string kTestDir = std::getenv("TEST_SRCDIR");
  const std::string kWorkdir = std::getenv("TEST_WORKSPACE");

  const std::string kBase =
      absl::StrJoin({kTestDir, kWorkdir, kTestDataLocation}, "/");
  config.add_auto_production(absl::StrJoin({kBase, kAutoProd}, "/"));
  config.add_production_chains(absl::StrJoin({kBase, kChains}, "/"));
  config.add_trade_goods(absl::StrJoin({kBase, kTradeGoods}, "/"));
  config.add_consumption(absl::StrJoin({kBase, kConsumption}, "/"));
  config.add_unit_templates(absl::StrJoin({kBase, kUnits}, "/"));
  config.set_world_file(absl::StrJoin({kBase, kWorld}, "/"));

  auto status = games::setup::LoadScenario(config, &scenario);
  EXPECT_TRUE(status.ok()) << status.error_message();

  status = games::setup::LoadWorld(config, &gameworld);
  EXPECT_TRUE(status.ok()) << status.error_message();

  auto world = games::setup::World::FromProto(gameworld);
  EXPECT_TRUE(world != NULL) << "Did not create world object";

  games::setup::proto::GameWorld save;
  status = world->ToProto(&save);
  EXPECT_TRUE(status.ok()) << status.error_message();

  google::protobuf::util::MessageDifferencer differ;
  EXPECT_TRUE(differ.Equals(gameworld, save))
      << gameworld.DebugString() << "\n\ndiffers from\n"
      << save.DebugString();
}
