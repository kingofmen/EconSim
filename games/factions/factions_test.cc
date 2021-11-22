#include "games/factions/factions.h"

#include <cmath>

#include "absl/strings/substitute.h"
#include "gtest/gtest.h"
#include "util/arithmetic/bits.h"
#include "util/logging/logging.h"
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

TEST(Faction, TestCombatSetup) {
  Log::Register(Log::coutLogger);
  struct TestCase {
    std::string desc;
    std::vector<util::proto::ObjectId> factions;
    util::objectid::Predicate willAlly;
    util::objectid::Predicate willFight;
    std::vector<bits::Mask> allies;
    std::vector<bits::Mask> enemies;
  };

  auto cases = std::vector<TestCase>({
      {
          "One vs one",
          {util::objectid::New("f", 0), util::objectid::New("f", 1)},
          util::objectid::Equal,
          util::objectid::NotEqual,
          {bits::GetMask(0)},
          {bits::GetMask(1)},
      },
      {
          "Three-way",
          {util::objectid::New("f", 0), util::objectid::New("f", 1),
           util::objectid::New("f", 2)},
          util::objectid::Equal,
          util::objectid::NotEqual,
          {bits::GetMask(0), bits::GetMask(0), bits::GetMask(1)},
          {bits::GetMask(1), bits::GetMask(2), bits::GetMask(2)},
      },
      {
          "Two alliances",
          {util::objectid::New("f", 0), util::objectid::New("f", 1),
           util::objectid::New("f", 2), util::objectid::New("f", 3),
           util::objectid::New("f", 4), util::objectid::New("f", 5)},
          [](const util::proto::ObjectId& one,
             const util::proto::ObjectId& two) {
            return (one.number() % 2 == two.number() % 2);
          },
          [](const util::proto::ObjectId& one,
             const util::proto::ObjectId& two) {
            return (one.number() % 2 != two.number() % 2);
          },
          {bits::GetMask(0, 2, 4)},
          {bits::GetMask(1, 3, 5)},
      },
      {
          "Triangle",
          {util::objectid::New("f", 0), util::objectid::New("f", 1),
           util::objectid::New("f", 2)},
          util::objectid::Equal,
          [](const util::proto::ObjectId& one,
             const util::proto::ObjectId& two) {
            return one.number() == 0 || two.number() == 0;
          },
          {bits::GetMask(0), bits::GetMask(0)},
          {bits::GetMask(1), bits::GetMask(2)},
      },
      {
          "Fight with bystander",
          {util::objectid::New("f", 0), util::objectid::New("f", 1),
           util::objectid::New("f", 2)},
          util::objectid::Equal,
          [](const util::proto::ObjectId& one,
             const util::proto::ObjectId& two) {
            return one.number() + two.number() == 1;
          },
          {bits::GetMask(0)},
          {bits::GetMask(1)},
      },
      {
          "Three separate battles",
          {util::objectid::New("f", 0), util::objectid::New("f", 1),
           util::objectid::New("f", 2), util::objectid::New("f", 3),
           util::objectid::New("f", 4), util::objectid::New("f", 5)},
          util::objectid::Equal,
          [](const util::proto::ObjectId& one,
             const util::proto::ObjectId& two) {
            return one.number() + two.number() == 5;
          },
          {bits::GetMask(0), bits::GetMask(1), bits::GetMask(2)},
          {bits::GetMask(5), bits::GetMask(4), bits::GetMask(3)},
      },
      {
          "No hostilities",
          {util::objectid::New("f", 0), util::objectid::New("f", 1),
           util::objectid::New("f", 2), util::objectid::New("f", 3),
           util::objectid::New("f", 4), util::objectid::New("f", 5)},
          util::objectid::Equal,
          util::objectid::Never,
          {},
          {},
      },
      {
          "All against All",
          {util::objectid::New("f", 0), util::objectid::New("f", 1),
           util::objectid::New("f", 2), util::objectid::New("f", 3),
           util::objectid::New("f", 4), util::objectid::New("f", 5)},
          util::objectid::Equal,
          util::objectid::NotEqual,
          {bits::GetMask(0), bits::GetMask(0), bits::GetMask(0),
           bits::GetMask(0), bits::GetMask(0), bits::GetMask(1),
           bits::GetMask(1), bits::GetMask(1), bits::GetMask(1),
           bits::GetMask(2), bits::GetMask(2), bits::GetMask(2),
           bits::GetMask(3), bits::GetMask(3), bits::GetMask(4)},
          {bits::GetMask(1), bits::GetMask(2), bits::GetMask(3),
           bits::GetMask(4), bits::GetMask(5), bits::GetMask(2),
           bits::GetMask(3), bits::GetMask(4), bits::GetMask(5),
           bits::GetMask(3), bits::GetMask(4), bits::GetMask(5),
           bits::GetMask(4), bits::GetMask(5), bits::GetMask(5)},
      },
      {
          "Two disjoint sides",
          {util::objectid::New("f", 0), util::objectid::New("f", 1),
           util::objectid::New("f", 2), util::objectid::New("f", 3),
           util::objectid::New("f", 4), util::objectid::New("f", 5)},
          [](const util::proto::ObjectId& one,
             const util::proto::ObjectId& two) {
            auto diff = abs((int) one.number() - (int) two.number());
            if (diff != 1) {
              return false;
            }
            if (one.number() * two.number() == 6) {
              return false;
            }
            return true;
          },
          [](const util::proto::ObjectId& one,
             const util::proto::ObjectId& two) {
            if (one.number() < 3 && two.number() > 2) {
              return true;
            }
            if (two.number() < 3 && one.number() > 2) {
              return true;
            }
            return false;
          },
          {bits::GetMask(0, 1), bits::GetMask(0, 1), bits::GetMask(1, 2),
           bits::GetMask(1, 2)},
          {bits::GetMask(3, 4), bits::GetMask(4, 5), bits::GetMask(3, 4),
           bits::GetMask(4, 5)},
      },
      {
          "Chaos",
          {util::objectid::New("f", 0), util::objectid::New("f", 1),
           util::objectid::New("f", 2), util::objectid::New("f", 3),
           util::objectid::New("f", 6), util::objectid::New("f", 7),
           util::objectid::New("f", 8), util::objectid::New("f", 9),
           util::objectid::New("f", 4), util::objectid::New("f", 5)},
          [](const util::proto::ObjectId& one,
             const util::proto::ObjectId& two) {
            if (util::objectid::Equal(one, two)) {
              return true;
            }
            auto low = one.number();
            auto high = two.number();
            if (low > high) {
              low = two.number();
              high = one.number();
            }
            if (low == 0) {
              return high == 1;
            }
            if (low == 2) {
              return high == 9;
            }
            if (low == 3) {
              return high == 4;
            }
            if (low == 4) {
              return high == 7;
            }
            return false;
          },
          [](const util::proto::ObjectId& one,
             const util::proto::ObjectId& two) {
            if (util::objectid::Equal(one, two)) {
              return false;
            }
            auto low = one.number();
            auto high = two.number();
            if (low > high) {
              low = two.number();
              high = one.number();
            }
            if (low == 0) {
              return high == 2 || high == 6 || high == 9;
            }
            if (low == 1) {
              return high == 2 || high == 3 || high == 4 || high == 7;
            }
            return false;
          },
          {bits::GetMask(0, 1), bits::GetMask(0), bits::GetMask(0),
           bits::GetMask(1), bits::GetMask(1)},
          {bits::GetMask(2), bits::GetMask(6), bits::GetMask(2, 9),
           bits::GetMask(3, 4), bits::GetMask(4, 7)},
      },
  });

  for (const auto& cc : cases) {
    Log::Infof("Starting Divide test: %s", cc.desc);
    auto conflicts = Divide(cc.factions, cc.willAlly, cc.willFight);
    EXPECT_EQ(conflicts.size(), cc.allies.size());
    if (conflicts.size() != cc.allies.size()) {
      for (const auto& conflict : conflicts) {
        Log::Infof("  %s vs %s", util::objectid::DisplayString(conflict.first),
                   util::objectid::DisplayString(conflict.second));
      }
    }
    for (unsigned int ii = 0; ii < conflicts.size(); ++ii) {
      const auto& conflict = conflicts[ii];
      auto allies = conflict.first;
      EXPECT_EQ(allies.size(), cc.allies[ii].count())
          << absl::Substitute("$0 conflict $1: Allies are $2", cc.desc, ii,
                              util::objectid::DisplayString(allies));
      for (const auto& ally : allies) {
        EXPECT_TRUE(cc.allies[ii].test(ally.number())) << absl::Substitute(
            "$0 conflict $1: Allies are $2, should be $3", cc.desc, ii,
            util::objectid::DisplayString(allies), cc.allies[ii].to_string());
      }
      auto enemies = conflict.second;
      EXPECT_EQ(enemies.size(), cc.enemies[ii].count());
      for (const auto& enemy : enemies) {
        EXPECT_TRUE(cc.enemies[ii].test(enemy.number())) << absl::Substitute(
            "$0 conflict $1: Enemies are $2, should be $3", cc.desc, ii,
            util::objectid::DisplayString(enemies), cc.enemies[ii].to_string());
      }
    }
  }
}

}  // namespace factions
