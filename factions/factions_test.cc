#include "factions/factions.h"

#include "gtest/gtest.h"
#include "util/proto/object_id.h"

namespace factions {

class FactionControllerTest : public testing::Test {
 protected:
  void SetUp() override {
    proto_.set_id(1);
  }

  proto::Faction proto_;
};

TEST_F(FactionControllerTest, TestCitizenship) {
  proto_.add_pop_ids(1);
  auto faction = FactionController::FromProto(proto_);
  EXPECT_TRUE(faction->IsFullCitizen(1));
  EXPECT_FALSE(faction->IsFullCitizen(2));
}

TEST_F(FactionControllerTest, TestPrivileges) {
  (*proto_.mutable_privileges())[1] =
      factions::proto::P_OVERRIDE_PRODUCTION | factions::proto::P_MIGRATE;
  (*proto_.mutable_privileges())[2] =
      factions::proto::P_OVERRIDE_PRODUCTION;

  auto faction = FactionController::FromProto(proto_);
  EXPECT_TRUE(faction->HasPrivileges(1, factions::proto::P_MIGRATE));
  EXPECT_TRUE(
      faction->HasPrivileges(1, factions::proto::P_OVERRIDE_PRODUCTION));
  EXPECT_TRUE(faction->HasPrivileges(
      1, factions::proto::P_OVERRIDE_PRODUCTION | factions::proto::P_MIGRATE));
  EXPECT_FALSE(faction->HasPrivileges(2, factions::proto::P_MIGRATE));
  EXPECT_TRUE(
      faction->HasPrivileges(2, factions::proto::P_OVERRIDE_PRODUCTION));
  EXPECT_FALSE(faction->HasPrivileges(
      2, factions::proto::P_OVERRIDE_PRODUCTION | factions::proto::P_MIGRATE));

  EXPECT_TRUE(faction->HasAnyPrivilege(1, factions::proto::P_MIGRATE));
  EXPECT_TRUE(
      faction->HasAnyPrivilege(1, factions::proto::P_OVERRIDE_PRODUCTION));
  EXPECT_TRUE(faction->HasAnyPrivilege(
      1, factions::proto::P_OVERRIDE_PRODUCTION | factions::proto::P_MIGRATE));
  EXPECT_FALSE(faction->HasAnyPrivilege(2, factions::proto::P_MIGRATE));
  EXPECT_TRUE(
      faction->HasAnyPrivilege(2, factions::proto::P_OVERRIDE_PRODUCTION));
  EXPECT_TRUE(faction->HasAnyPrivilege(
      2, factions::proto::P_OVERRIDE_PRODUCTION | factions::proto::P_MIGRATE));
}

TEST_F(FactionControllerTest, TestObjectId) {
  proto_.clear_id();
  proto_.mutable_faction_id()->set_type(1);
  proto_.mutable_faction_id()->set_number(1);
  proto_.mutable_faction_id()->set_tag("one");

  auto faction = FactionController::FromProto(proto_);
  EXPECT_NE(faction.get(), nullptr);
  auto* lookup = FactionController::GetByID(proto_.faction_id());
  EXPECT_EQ(faction.get(), lookup);
  std::equal_to<util::proto::ObjectId> equals;
  EXPECT_TRUE(equals(proto_.faction_id(), faction->faction_id()))
      << "Not equal: " << proto_.faction_id().DebugString() << "\nvs\n"
      << faction->faction_id().DebugString();
}

}  // namespace factions
