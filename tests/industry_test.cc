#include "industry/industry.h"

#include "industry/proto/industry.pb.h"
#include "market/goods_utils.h"
#include "market/proto/goods.pb.h"
#include "gtest/gtest.h"

namespace industry {

using market::proto::Container;

TEST(IndustryTest, OneStep) {
  Production production;
  Container inputs;
  Container outputs;
  production.PerformStep(&inputs, &outputs);
  EXPECT_TRUE(production.Complete());
}

} // namespace industry
