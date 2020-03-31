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
  google::protobuf::util::MessageDifferencer differ;

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
  games::setup::Constants constants(scenario);
  // Add better tests here.
  ASSERT_EQ(constants.auto_production_.size(), scenario.auto_production_size());
  for (int i = 0; i < constants.auto_production_.size(); ++i) {
    const auto* loaded = constants.auto_production_[i];
    const auto& original = scenario.auto_production(i);
    EXPECT_TRUE(differ.Equals(*loaded, original))
        << loaded->DebugString() << "\n\ndiffers from\n"
        << original.DebugString();
  }
  ASSERT_EQ(constants.production_chains_.size(), scenario.production_chains_size());
  for (int i = 0; i < constants.production_chains_.size(); ++i) {
    const auto* loaded = constants.production_chains_[i];
    const auto& original = scenario.production_chains(i);
    EXPECT_TRUE(differ.Equals(*loaded, original))
        << loaded->DebugString() << "\n\ndiffers from\n"
        << original.DebugString();
  }
  for (const auto& temp_proto : scenario.unit_templates()) {
    auto* temp = units::Unit::TemplateById(temp_proto.id());
    EXPECT_TRUE(temp != NULL)
        << "No template created for " << temp_proto.DebugString();
  }

  status = games::setup::LoadWorld(config, &gameworld);
  EXPECT_TRUE(status.ok()) << status.error_message();

  auto world = games::setup::World::FromProto(gameworld);
  EXPECT_TRUE(world != NULL) << "Did not create world object";

  games::setup::proto::GameWorld save;
  status = world->ToProto(&save);
  EXPECT_TRUE(status.ok()) << status.error_message();

  EXPECT_TRUE(differ.Equals(gameworld, save))
      << gameworld.DebugString() << "\n\ndiffers from\n"
      << save.DebugString();
}

TEST(SetupTest, TestCanonicaliseAndRestoreTags) {
  games::setup::proto::GameWorld gameworld;
  auto* faction = gameworld.add_factions();
  faction->mutable_faction_id()->set_type(1);
  faction->mutable_faction_id()->set_number(1);
  faction->mutable_faction_id()->set_tag("faction_one");

  auto* area = gameworld.add_areas();
  area->mutable_area_id()->set_type(2);
  area->mutable_area_id()->set_number(51);
  area->mutable_area_id()->set_tag("area_fifty_one");

  // Test canonicalisation.
  auto status = games::setup::CanonicaliseWorld(&gameworld);
  EXPECT_TRUE(status.ok()) << status.error_message();
  EXPECT_TRUE(area->area_id().tag().empty()) << area->DebugString();
  EXPECT_TRUE(faction->faction_id().tag().empty()) << faction->DebugString();

  // Test lookups.
  auto world = games::setup::World::FromProto(gameworld);
  ASSERT_EQ(1, world->areas_.size());
  auto* alookup = geography::Area::GetById(area->area_id());
  EXPECT_EQ(world->areas_[0].get(), alookup);

  ASSERT_EQ(1, world->factions_.size());
  auto* flookup = factions::FactionController::GetByID(faction->faction_id());
  EXPECT_EQ(world->factions_[0].get(), flookup);

  // Test restoration.
  gameworld.Clear();
  status = world->ToProto(&gameworld);
  EXPECT_EQ(gameworld.factions(0).faction_id().tag(), "faction_one");
  EXPECT_EQ(gameworld.factions(0).faction_id().number(), 1);
  EXPECT_EQ(gameworld.areas(0).area_id().tag(), "area_fifty_one");
  EXPECT_EQ(gameworld.areas(0).area_id().number(), 51);


}
