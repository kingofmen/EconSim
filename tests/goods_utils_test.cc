#include "goods_utils.h"

#include "gtest/gtest.h"
#include "proto/goods.pb.h"

namespace market {

TEST(GoodsUtilsTest, Combine) {
  Quantity dis;
  dis.set_kind("TestGood");
  dis.set_amount(1.0);
  Quantity dat;
  dat.set_kind("TestGood");
  dat.set_amount(1.0);
  combine(dis, &dat);
  EXPECT_DOUBLE_EQ(2.0, dat.amount());
  EXPECT_DOUBLE_EQ(1.0, dis.amount());
}

} // namespace market


