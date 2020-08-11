#ifndef GAMES_AI_IMPL_AI_TESTING_HH
#define GAMES_AI_IMPL_AI_TESTING_HH

#include "games/actions/proto/plan.pb.h"
#include "games/actions/proto/strategy.pb.h"
#include "games/geography/connection.h"
#include "games/geography/geography.h"
#include "games/units/unit.h"
#include "gtest/gtest.h"

namespace ai {

// Base class that does common setup for AI test frameworks.
class AiTestBase : public testing::Test {
 protected:
  void SetUp() override;
  void TearDown() override;

  std::unique_ptr<geography::Connection> connection_12;
  std::unique_ptr<geography::Connection> connection_14;
  std::unique_ptr<geography::Connection> connection_23;
  std::unique_ptr<geography::Area> area1_;
  std::unique_ptr<geography::Area> area2_;
  std::unique_ptr<geography::Area> area3_;
  std::unique_ptr<geography::Area> area4_;
  std::unique_ptr<units::Unit> unit_;
  actions::proto::Strategy strategy_;
  actions::proto::Plan plan_;
};

} // namespace ai




#endif
