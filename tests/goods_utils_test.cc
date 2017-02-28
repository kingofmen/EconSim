#include "goods_utils.h"

#include <string>

#include "proto/goods.pb.h"
#include "gtest/gtest.h"

namespace market {

namespace {
constexpr char kTestGood1[] = "TestGood1";
constexpr char kTestGood2[] = "TestGood2";
}

using proto::Quantity;
using proto::Container;

TEST(GoodsUtilsTest, HelperFunctions) {
  Container container;
  Quantity quantity;
  quantity.set_kind(kTestGood1);
  container += quantity;

  EXPECT_TRUE(Contains(container, kTestGood1));
  EXPECT_TRUE(Contains(container, quantity));
  EXPECT_FALSE(Contains(container, kTestGood2));
  EXPECT_DOUBLE_EQ(GetAmount(container, kTestGood1), 0);
  EXPECT_DOUBLE_EQ(GetAmount(container, kTestGood2), 0);

  quantity += 1;
  container += quantity;
  EXPECT_DOUBLE_EQ(GetAmount(container, kTestGood1), 1);
  EXPECT_DOUBLE_EQ(GetAmount(container, quantity), 1);

  Clear(container);
  EXPECT_TRUE(Contains(container, kTestGood1));
  EXPECT_DOUBLE_EQ(GetAmount(container, kTestGood1), 0);
}

TEST(GoodsUtilsTest, StreamOperators) {
  Container container;
  container << kTestGood1;
  EXPECT_TRUE(Contains(container, kTestGood1));
  EXPECT_FALSE(Contains(container, kTestGood2));
  container << kTestGood1;
  EXPECT_TRUE(Contains(container, kTestGood1));
  EXPECT_FALSE(Contains(container, kTestGood2));

  Quantity quantity;
  quantity.set_kind(kTestGood2);
  quantity += 1;
  container << quantity;
  EXPECT_DOUBLE_EQ(GetAmount(container, kTestGood2), 1);
  EXPECT_DOUBLE_EQ(quantity.amount(), 0);

  container >> quantity;
  EXPECT_DOUBLE_EQ(GetAmount(container, kTestGood2), 0);
  EXPECT_DOUBLE_EQ(quantity.amount(), 1);
}

TEST(GoodsUtilsTest, PlusAndMinus) {
  Quantity quantity;
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
