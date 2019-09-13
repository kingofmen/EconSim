#include "factions/factions.h"

#include "gtest/gtest.h"

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

}  // namespace factions
