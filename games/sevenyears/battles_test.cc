#include "games/sevenyears/battles.h"

#include <string>
#include <vector>

#include "games/units/unit.h"
#include "games/setup/proto/setup.pb.h"
#include "games/setup//setup.h"
#include "games/sevenyears/test_utils.h"
#include "gtest/gtest.h"
#include "util/logging/logging.h"

namespace sevenyears {

class ObserverTest : public testing::Test, public TestState {
 protected:
  ObserverTest() { Log::Register(Log::coutLogger); }
  ~ObserverTest() { Log::UnRegister(Log::coutLogger); }

  void SetUp() override {
    units::Unit::ClearTemplates();
  }
};

TEST_F(ObserverTest, Interception) {
  struct testCase {
    testCase(std::string d, const util::proto::ObjectId& area1,
             micro::Measure s1_u, micro::Measure d1_u,
             const util::proto::ObjectId& area2, micro::Measure s2_u,
             micro::Measure d2_u, micro::Measure l_u, micro::Measure e_u)
        : desc(d), movement(util::objectid::kNullId, util::objectid::kNullId,
                            area1, s1_u, d1_u),
          otherMove(util::objectid::kNullId, util::objectid::kNullId, area2,
                    s2_u, d2_u),
          length_u(l_u), expect_u(e_u) {}
    std::string desc;
    geography::Connection::Movement movement;
    geography::Connection::Movement otherMove;
    micro::Measure length_u;
    micro::Measure expect_u;
  };

  std::vector<testCase> cases = {
      {"opposite directions, interception at end",
       util::objectid::New("area", 1), micro::kZeroInU, micro::kHalfInU,
       util::objectid::New("area", 2), micro::kZeroInU, micro::kHalfInU,
       micro::kOneInU, micro::kHalfInU},
      {"opposite directions, interception at start",
       util::objectid::New("area", 1), micro::kHalfInU, micro::kHalfInU,
       util::objectid::New("area", 2), micro::kHalfInU, micro::kHalfInU,
       micro::kOneInU, micro::kHalfInU},
      {"opposite directions, different speeds, intercept in middle",
       util::objectid::New("area", 1), micro::kZeroInU, micro::kHalfInU,
       util::objectid::New("area", 2), micro::kZeroInU, micro::kOneInU,
       micro::kOneInU, micro::kOneThirdInU},
      {"opposite directions, insufficient speeds, interception out of range",
       util::objectid::New("area", 1), micro::kZeroInU, micro::kHalfInU,
       util::objectid::New("area", 2), micro::kZeroInU, micro::kHalfInU,
       2 * micro::kOneInU, micro::kOneInU},
      {"opposite directions, start passed, negative interception",
       util::objectid::New("area", 1), micro::kTwoThirdsInU, micro::kHalfInU,
       util::objectid::New("area", 2), micro::kTwoThirdsInU, micro::kHalfInU,
       2 * micro::kOneInU, micro::kOneInU},
      {"same directions, equal speed, intercept at start",
       util::objectid::New("area", 1), micro::kTwoThirdsInU, micro::kHalfInU,
       util::objectid::New("area", 1), micro::kTwoThirdsInU, micro::kHalfInU,
       2 * micro::kOneInU, micro::kTwoThirdsInU},
      {"same directions, equal speed, no interception",
       util::objectid::New("area", 1), micro::kTwoThirdsInU, micro::kHalfInU,
       util::objectid::New("area", 1), micro::kTwoThirdsInU + 1,
       micro::kHalfInU, 2 * micro::kOneInU, -1},
      {"same directions, double speed, interception in middle",
       util::objectid::New("area", 1), micro::kZeroInU, micro::kOneInU,
       util::objectid::New("area", 1), micro::kOneFourthInU, micro::kHalfInU,
       micro::kOneInU, micro::kHalfInU},
      {"same directions, half speed, negative interception",
       util::objectid::New("area", 1), micro::kZeroInU, micro::kHalfInU,
       util::objectid::New("area", 1), micro::kOneFourthInU, micro::kOneInU,
       micro::kOneInU, -micro::kOneFourthInU},
  };

  for (const auto& cc : cases) {
    auto got = Interception(cc.movement, cc.otherMove, cc.length_u);
    EXPECT_EQ(got, cc.expect_u) << cc.desc;
  }
}

TEST_F(ObserverTest, DetectMeetings) {
  games::setup::proto::ScenarioFiles config;
  PopulateScenarioFiles("battles", &config);
  auto status = PopulateProtos(config);
  ASSERT_TRUE(status.ok()) << status.error_message();
  ASSERT_EQ(world_proto_.units_size(), 2);
  ASSERT_EQ(world_proto_.connections_size(), 1);

  struct testCase {
    std::vector<micro::Measure> start1_us;
    std::vector<micro::Measure> dist1_us;
    std::vector<micro::Measure> start2_us;
    std::vector<micro::Measure> dist2_us;
    std::vector<micro::Measure> want_us;
    bool same;
  };

  std::vector<testCase> cases = {
      {
          {micro::kZeroInU},
          {micro::kHalfInU},
          {micro::kZeroInU},
          {micro::kHalfInU},
          {micro::kHalfInU},
          false,
      },
      {
          {micro::kZeroInU, micro::kHalfInU},
          {micro::kHalfInU, micro::kHalfInU},
          {micro::kOneTenthInU*8, micro::kZeroInU},
          {micro::kZeroInU, micro::kHalfInU},
          {micro::kOneTenthInU*2, micro::kThreeFourthsInU},
          false,
      },
      {
          {micro::kZeroInU},
          {micro::kHalfInU},
          {micro::kZeroInU},
          {micro::kHalfInU},
          {micro::kZeroInU},
          true,
      },
      {
          {micro::kZeroInU},
          {micro::kHalfInU},
          {micro::kHalfInU},
          {micro::kHalfInU},
          {},
          true,
      },
  };

  class TestResolver : public BattleResolver {
   public:
    void Resolve(Encounter& encounter) override {
      encounters.push_back(encounter.point_u);
    }
    std::vector<micro::Measure> encounters;
  };

  const auto connection =
      geography::Connection::FromProto(world_proto_.connections(0));
  const auto connection_id = connection->connection_id();
  std::vector<std::unique_ptr<units::Unit>> units;
  for (const auto& proto : world_proto_.units()) {
    units.emplace_back(units::Unit::FromProto(proto));
  }
  const auto area_1_id = util::objectid::New("area", 1);
  const auto area_2_id = util::objectid::New("area", 2);
  const auto& unit_1_id = world_proto_.units(0).unit_id();
  const auto& unit_2_id = world_proto_.units(1).unit_id();
  LandMoveObserver observer;
  TestResolver resolver;
  for (const auto& cc : cases) {
    observer.Clear();
    resolver.encounters.clear();
    for (unsigned int i = 0; i < cc.start1_us.size(); ++i) {
      geography::Connection::Movement movement(
          connection_id, unit_1_id, area_1_id, cc.start1_us[i], cc.dist1_us[i]);
      observer.Listen(movement);
    }
    for (unsigned int i = 0; i < cc.start2_us.size(); ++i) {
      geography::Connection::Movement movement(connection_id, unit_2_id,
                                               cc.same ? area_1_id : area_2_id,
                                               cc.start2_us[i], cc.dist2_us[i]);
      observer.Listen(movement);
    }

    observer.Battle(resolver);

    if (resolver.encounters.size() != cc.want_us.size()) {
      EXPECT_EQ(cc.want_us.size(), resolver.encounters.size());
      continue;
    }
    for (unsigned int idx = 0; idx < resolver.encounters.size(); ++idx) {
      EXPECT_EQ(cc.want_us[idx], resolver.encounters[idx]);
    }
  }
}

} // namespace sevenyears
