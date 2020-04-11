#include "games/setup/setup.h"

#include "absl/strings/str_join.h"
#include "gtest/gtest.h"
#include "src/google/protobuf/util/message_differencer.h"
#include "util/logging/logging.h"

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
    auto* temp = units::Unit::TemplateById(temp_proto.template_id());
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
  Log::Register(Log::coutLogger);

  games::setup::proto::Scenario scenario;
  auto* temp = scenario.add_unit_templates();
  temp->mutable_template_id()->set_kind("unit");
  games::setup::Constants constants(scenario);

  games::setup::proto::GameWorld gameworld;
  auto* faction = gameworld.add_factions();
  faction->mutable_faction_id()->set_type(1);
  faction->mutable_faction_id()->set_number(1);
  faction->mutable_faction_id()->set_tag("faction_one");

  auto* area51 = gameworld.add_areas();
  area51->mutable_area_id()->set_kind("area");
  area51->mutable_area_id()->set_number(51);
  area51->mutable_area_id()->set_tag("area_fifty_one");

  auto* area101 = gameworld.add_areas();
  area101->mutable_area_id()->set_kind("area");
  area101->mutable_area_id()->set_number(101);
  area101->mutable_area_id()->set_tag("area_one_oh_one");

  auto* conn = gameworld.add_connections();
  conn->set_id(1);
  conn->set_distance_u(1);
  conn->set_width_u(1);
  conn->mutable_a_area_id()->set_kind("area");
  conn->mutable_a_area_id()->set_tag("area_fifty_one");
  conn->mutable_z_area_id()->set_kind("area");
  conn->mutable_z_area_id()->set_number(101);

  auto* unit = gameworld.add_units();
  unit->mutable_unit_id()->set_kind("unit");
  unit->mutable_unit_id()->set_number(1);
  auto* loc = unit->mutable_location();
  loc->mutable_a_area_id()->set_kind("area");
  loc->mutable_a_area_id()->set_tag("area_fifty_one");
  loc->mutable_z_area_id()->set_kind("area");
  loc->mutable_z_area_id()->set_tag("area_one_oh_one");
  loc->set_connection_id(1);

  // Test canonicalisation.
  auto status = games::setup::CanonicaliseWorld(&gameworld);
  EXPECT_TRUE(status.ok()) << status.error_message();
  // Tags are all empty after canonicalisation.
  EXPECT_TRUE(area51->area_id().tag().empty()) << area51->DebugString();
  EXPECT_TRUE(area101->area_id().tag().empty()) << area101->DebugString();
  EXPECT_TRUE(faction->faction_id().tag().empty()) << faction->DebugString();
  EXPECT_TRUE(conn->a_area_id().tag().empty()) << conn->DebugString();
  EXPECT_TRUE(conn->z_area_id().tag().empty()) << conn->DebugString();
  EXPECT_TRUE(loc->a_area_id().tag().empty()) << conn->DebugString();
  EXPECT_TRUE(loc->z_area_id().tag().empty()) << conn->DebugString();

  // Canonical ObjectIds can be compared for equality.
  EXPECT_TRUE(conn->a_area_id() == area51->area_id()) << conn->DebugString();
  EXPECT_TRUE(conn->z_area_id() == area101->area_id()) << conn->DebugString();
  EXPECT_TRUE(loc->a_area_id() == area51->area_id()) << unit->DebugString();
  EXPECT_TRUE(loc->z_area_id() == area101->area_id()) << unit->DebugString();

  // Test lookups.
  auto world = games::setup::World::FromProto(gameworld);
  ASSERT_EQ(2, world->areas_.size());
  auto* alookup = geography::Area::GetById(area51->area_id());
  EXPECT_EQ(world->areas_[0].get(), alookup);
  alookup = geography::Area::GetById(area101->area_id());
  EXPECT_EQ(world->areas_[1].get(), alookup);

  ASSERT_EQ(1, world->factions_.size());
  auto* flookup = factions::FactionController::GetByID(faction->faction_id());
  EXPECT_EQ(world->factions_[0].get(), flookup);

  ASSERT_EQ(1, world->connections_.size());
  ASSERT_EQ(1, world->units_.size());

  // Test restoration.
  gameworld.Clear();
  status = world->ToProto(&gameworld);
  EXPECT_TRUE(status.ok()) << status.error_message();
  EXPECT_EQ(gameworld.factions(0).faction_id().tag(), "faction_one");
  EXPECT_EQ(gameworld.factions(0).faction_id().number(), 1);
  EXPECT_EQ(gameworld.areas(0).area_id().tag(), "area_fifty_one");
  EXPECT_EQ(gameworld.areas(0).area_id().number(), 51);
  EXPECT_EQ(gameworld.areas(1).area_id().tag(), "area_one_oh_one");
  EXPECT_EQ(gameworld.areas(1).area_id().number(), 101);

  const auto& conn_a = gameworld.connections(0).a_area_id();
  const auto& conn_z = gameworld.connections(0).z_area_id();
  EXPECT_FALSE(conn_a.has_number()) << conn_a.DebugString();
  EXPECT_FALSE(conn_z.has_number()) << conn_z.DebugString();
  EXPECT_EQ(conn_a.tag(), "area_fifty_one");
  EXPECT_EQ(conn_z.tag(), "area_one_oh_one");

  const auto& unit_a = gameworld.units(0).location().a_area_id();
  const auto& unit_z = gameworld.units(0).location().z_area_id();
  EXPECT_FALSE(unit_a.has_number()) << unit_a.DebugString();
  EXPECT_FALSE(unit_z.has_number()) << unit_z.DebugString();
  EXPECT_EQ(unit_a.tag(), "area_fifty_one");
  EXPECT_EQ(unit_z.tag(), "area_one_oh_one");
}
