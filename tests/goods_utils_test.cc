#include "goods_utils.h"

#include <string>

#include "proto/goods.pb.h"
#include "gtest/gtest.h"

namespace market {

TEST(GoodsUtilsTest, PlusAndMinus) {
  Quantity quantity;
  const std::string kTestGood1("TestGood1");
  const std::string kTestGood2("TestGood2");
  quantity.set_kind(kTestGood1);
  quantity += 1;
  EXPECT_DOUBLE_EQ(1.0, quantity.amount());

  quantity -= 0.5;
  EXPECT_DOUBLE_EQ(0.5, quantity.amount());

  Container container;
  container += quantity;
  container += quantity;
  EXPECT_DOUBLE_EQ(1.0, container.quantities().at(quantity.kind()).amount());
  container -= quantity;
  EXPECT_DOUBLE_EQ(0.5, container.quantities().at(quantity.kind()).amount());

  Quantity another;
  another.set_kind(kTestGood2);
  another += 1;
  container += another;
  EXPECT_DOUBLE_EQ(0.5, container.quantities().at(quantity.kind()).amount());
  EXPECT_DOUBLE_EQ(another.amount(),
                   container.quantities().at(another.kind()).amount());

  Container barrel;
  barrel += container;
  EXPECT_DOUBLE_EQ(container.quantities().at(kTestGood1).amount(),
                   barrel.quantities().at(kTestGood1).amount());
  EXPECT_DOUBLE_EQ(container.quantities().at(kTestGood2).amount(),
                   barrel.quantities().at(kTestGood2).amount());

  Container box = barrel + container;
  EXPECT_DOUBLE_EQ(container.quantities().at(kTestGood1).amount() +
                       barrel.quantities().at(kTestGood1).amount(),
                   box.quantities().at(kTestGood1).amount());
  EXPECT_DOUBLE_EQ(container.quantities().at(kTestGood2).amount() +
                       barrel.quantities().at(kTestGood2).amount(),
                   box.quantities().at(kTestGood2).amount());
  box -= barrel;
  EXPECT_DOUBLE_EQ(container.quantities().at(kTestGood1).amount(),
                   box.quantities().at(kTestGood1).amount());
  EXPECT_DOUBLE_EQ(container.quantities().at(kTestGood2).amount(),
                   box.quantities().at(kTestGood2).amount());

  Container jar = barrel - container;
  EXPECT_DOUBLE_EQ(container.quantities().at(kTestGood1).amount() -
                       barrel.quantities().at(kTestGood1).amount(),
                   jar.quantities().at(kTestGood1).amount());
  EXPECT_DOUBLE_EQ(container.quantities().at(kTestGood2).amount() -
                       barrel.quantities().at(kTestGood2).amount(),
                   jar.quantities().at(kTestGood2).amount());
}

} // namespace market


