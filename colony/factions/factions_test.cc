#include "colony/factions/factions.h"

#include "gtest/gtest.h"

namespace factions {

class FactionControllerTest : public testing::Test {
 protected:
  void SetUp() override {}

  proto::Faction proto_;
};

TEST_F(FactionControllerTest, TestCitizenship) {
  proto_.add_pop_ids(1);
  FactionController faction(proto_);
  EXPECT_TRUE(faction.IsFullCitizen(1));
  EXPECT_FALSE(faction.IsFullCitizen(2));
}

}  // namespace factions
