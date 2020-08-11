#include "games/ai/impl/ai_utils.h"

#include "games/ai/impl/ai_testing.h"
#include "games/geography/connection.h"
#include "games/units/unit.h"
#include "games/units/proto/templates.pb.h"
#include "games/units/proto/units.pb.h"
#include "gtest/gtest.h"

namespace ai {
namespace utils {

class AiUtilsTest : public AiTestBase {};

TEST_F(AiUtilsTest, TestNumTurns) {
  std::vector<uint64> path;
  path.push_back(connection_12->connection_id());
  EXPECT_EQ(1, NumTurns(*unit_, path));
}

} // namespace impl
} // namespace ai
