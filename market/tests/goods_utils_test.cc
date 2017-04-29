#include "market/goods_utils.h"

#include <string>

#include "market/proto/goods.pb.h"
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

  Clear(&container);
  EXPECT_FALSE(Contains(container, kTestGood1));
  EXPECT_DOUBLE_EQ(GetAmount(container, kTestGood1), 0);

  SetAmount(kTestGood1, 1, &container);
  EXPECT_DOUBLE_EQ(GetAmount(container, kTestGood1), 1);

  quantity.set_amount(3);
  SetAmount(quantity, &container);
  EXPECT_DOUBLE_EQ(GetAmount(container, kTestGood1), 3);

  Add(kTestGood1, 1, &container);
  EXPECT_DOUBLE_EQ(GetAmount(container, kTestGood1), 4);

  Container container2;
  Move(kTestGood1, 1, &container, &container2);
  EXPECT_DOUBLE_EQ(GetAmount(container, kTestGood1), 3);
  EXPECT_DOUBLE_EQ(GetAmount(container2, kTestGood1), 1);
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
  EXPECT_DOUBLE_EQ(1.0, GetAmount(container, quantity));
  container -= quantity;
  EXPECT_DOUBLE_EQ(0.5, GetAmount(container, quantity));

  Quantity another;
  another.set_kind(kTestGood2);
  another += 1;
  container += another;
  EXPECT_DOUBLE_EQ(0.5, GetAmount(container, quantity));
  EXPECT_DOUBLE_EQ(another.amount(), GetAmount(container, another));

  Container barrel;
  barrel += container;
  EXPECT_DOUBLE_EQ(GetAmount(container, kTestGood1),
                   GetAmount(barrel, kTestGood1));
  EXPECT_DOUBLE_EQ(GetAmount(container, kTestGood2),
                   GetAmount(barrel, kTestGood2));

  Container box = barrel + container;
  EXPECT_DOUBLE_EQ(GetAmount(container, kTestGood1) +
                       GetAmount(barrel, kTestGood1),
                   GetAmount(box, kTestGood1));
  EXPECT_DOUBLE_EQ(GetAmount(container, kTestGood2) +
                       GetAmount(barrel, kTestGood2),
                   GetAmount(box, kTestGood2));

  box -= barrel;
  EXPECT_DOUBLE_EQ(GetAmount(container, kTestGood1),
                   GetAmount(box, kTestGood1));
  EXPECT_DOUBLE_EQ(GetAmount(container, kTestGood2),
                   GetAmount(box, kTestGood2));

  Container jar = barrel - container;
  EXPECT_DOUBLE_EQ(GetAmount(container, kTestGood1) -
                       GetAmount(barrel, kTestGood1),
                   GetAmount(jar, kTestGood1));
  EXPECT_DOUBLE_EQ(GetAmount(container, kTestGood2) -
                       GetAmount(barrel, kTestGood2),
                   GetAmount(jar, kTestGood2));
}

TEST(GoodsUtilsTest, Multiply) {
  Quantity quantity;
  quantity.set_kind(kTestGood1);
  quantity += 1;
  EXPECT_DOUBLE_EQ(1.0, quantity.amount());
  quantity *= 2;
  EXPECT_DOUBLE_EQ(2.0, quantity.amount());

  Container container;
  container += quantity;
  container *= 2;
  EXPECT_DOUBLE_EQ(4.0, GetAmount(container, quantity));

  auto new_container = container * 2;
  EXPECT_DOUBLE_EQ(8.0, GetAmount(new_container, quantity));

  auto new_quantity = quantity * 2;
  EXPECT_DOUBLE_EQ(4.0, new_quantity.amount());
}

TEST(GoodsUtilsTest, MatrixMultiply) {
  Quantity quantity1;
  quantity1.set_kind(kTestGood1);
  quantity1 += 1;

  Quantity quantity2;
  quantity2.set_kind(kTestGood2);
  quantity2 += 1;

  Container container1;
  container1 << quantity1;
  container1 << quantity2;

  Container container2;
  quantity1 += 0.5;
  container2 << quantity1;
  quantity2 += 1.5;
  container2 << quantity2;
 
  container1 *= container2;
  EXPECT_DOUBLE_EQ(GetAmount(container1, quantity1),
                   GetAmount(container2, quantity1));
  EXPECT_DOUBLE_EQ(GetAmount(container1, quantity2),
                   GetAmount(container2, quantity2));
}

TEST(GoodsUtilsTest, CleanContainer) {
  Quantity quantity1;
  quantity1.set_kind(kTestGood1);
  quantity1 += 1e-9;

  Quantity quantity2;
  quantity2.set_kind(kTestGood2);
  quantity2 += 1;

  Container container;
  container << quantity1;
  container << quantity2;
  EXPECT_TRUE(Contains(container, quantity1));

  CleanContainer(&container);
  EXPECT_FALSE(Contains(container, quantity1));
  EXPECT_DOUBLE_EQ(GetAmount(container, quantity2), 1);
}

TEST(GoodsUtilsTest, Iterator) {
  Quantity gold;
  gold.set_kind(kTestGood1);
  gold.set_amount(1);

  Container chest;
  chest << gold;
  EXPECT_DOUBLE_EQ(gold.amount(), 0);
  EXPECT_DOUBLE_EQ(GetAmount(chest, gold), 1.0);
  EXPECT_DOUBLE_EQ(GetAmount(chest, kTestGood1), 1.0);

  for (const auto& quantity : chest.quantities()) {
    EXPECT_EQ(kTestGood1, quantity.first);
    EXPECT_DOUBLE_EQ(1, quantity.second);
  }
}

TEST(GoodsUtilsTest, Relational) {
  Quantity gold;
  gold.set_kind(kTestGood1);
  gold.set_amount(1);
  Quantity salt;
  salt.set_kind(kTestGood2);
  salt.set_amount(1);

  Container chest;
  chest << gold;

  Container shaker;
  shaker << salt;

  EXPECT_FALSE(shaker < chest); // chest - shaker has negative salt.
  EXPECT_FALSE(shaker <= chest);
  EXPECT_FALSE(chest < shaker); // shaker - chest has negative gold.
  EXPECT_FALSE(chest <= shaker);

  EXPECT_FALSE(shaker > chest); // Does not have more gold.
  EXPECT_FALSE(shaker >= chest);

  EXPECT_FALSE(chest > shaker); // Does not have more salt.
  EXPECT_FALSE(chest >= shaker);

  Container bag;
  gold += 0.5;
  bag << gold;
  EXPECT_TRUE(chest > bag);
  EXPECT_TRUE(chest >= bag);
  EXPECT_FALSE(chest < bag);
  EXPECT_FALSE(chest <= bag);

  EXPECT_FALSE(bag > chest);
  EXPECT_FALSE(bag >= chest);
  EXPECT_TRUE(bag < chest);
  EXPECT_TRUE(bag <= chest);

  salt += 1.5;
  bag << salt;
  EXPECT_TRUE(bag > shaker);
  EXPECT_TRUE(bag >= shaker);
  EXPECT_FALSE(bag < shaker);
  EXPECT_FALSE(bag <= shaker);

  EXPECT_FALSE(shaker > bag);
  EXPECT_FALSE(shaker >= bag);
  EXPECT_TRUE(shaker < bag);
  EXPECT_TRUE(shaker <= bag);

  // The bag has less gold, but more salt, than the chest. So neither can be
  // safely subtracted from the other.
  EXPECT_FALSE(bag < chest); // chest - bag has negative salt.
  EXPECT_FALSE(chest < bag); // Negative gold.
  EXPECT_FALSE(bag > chest); // Less gold.
  EXPECT_FALSE(chest > bag); // Less salt.

  gold += 0.75;
  salt += 1.25;
  EXPECT_TRUE(bag < gold);
  EXPECT_FALSE(bag > gold);
  EXPECT_FALSE(bag < salt);
  EXPECT_TRUE(bag > salt);

  EXPECT_TRUE(chest < salt);
  EXPECT_TRUE(chest > gold);
  EXPECT_TRUE(shaker < gold);
  EXPECT_TRUE(shaker < salt);
}

TEST(GoodsUtilsTest, EmptyRelational) {
  Container empty1;
  Container empty2;
  EXPECT_TRUE(empty1 < empty2);
  EXPECT_TRUE(empty2 < empty1);
  EXPECT_TRUE(empty1 > empty2);
  EXPECT_TRUE(empty2 > empty1);
  EXPECT_TRUE(empty1 <= empty2);
  EXPECT_TRUE(empty2 <= empty1);
  EXPECT_TRUE(empty1 >= empty2);
  EXPECT_TRUE(empty2 >= empty1);
}

TEST(GoodsUtilsTest, DotProduct) {
  Quantity gold;
  gold.set_kind(kTestGood1);
  gold.set_amount(1);
  Quantity salt;
  salt.set_kind(kTestGood2);
  salt.set_amount(1);

  Container chest;
  chest << gold;

  Container shaker;
  shaker << salt;

  EXPECT_EQ(0, chest * shaker);

  gold += 2;
  shaker << gold;
  EXPECT_DOUBLE_EQ(2, chest * shaker);

  salt += 2;
  chest << salt;
  EXPECT_DOUBLE_EQ(4, chest * shaker);

  salt += 2;
  shaker << salt;
  EXPECT_DOUBLE_EQ(8, chest * shaker);
}

TEST(GoodsUtilsTest, TwoGoodsRelations) {
  Quantity gold;
  gold.set_kind(kTestGood1);
  gold.set_amount(1);
  Quantity salt;
  salt.set_kind(kTestGood2);
  salt.set_amount(1);
  Quantity fish;
  fish.set_kind("fish");
  fish.set_amount(1);

  Container chest;
  chest << gold;
  chest << salt;

  salt += 1;
  Container shaker;
  shaker << salt;
  shaker << fish;

  EXPECT_FALSE(chest < shaker);
}

} // namespace market
