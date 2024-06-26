#include "games/market/goods_utils.h"

#include <string>

#include "games/market/proto/goods.pb.h"
#include "util/arithmetic/microunits.h"
#include "gtest/include/gtest/gtest.h"

namespace market {

namespace {
constexpr char kTestGood1[] = "TestGood1";
constexpr char kTestGood2[] = "TestGood2";
}

using proto::Quantity;
using proto::Container;

TEST(GoodsUtilsTest, HelperFunctions) {
  Container container;
  EXPECT_TRUE(Empty(container));
  Quantity quantity;
  quantity.set_kind(kTestGood1);
  container += quantity;

  EXPECT_TRUE(Empty(container));
  EXPECT_TRUE(Contains(container, kTestGood1));
  EXPECT_TRUE(Contains(container, quantity));
  EXPECT_FALSE(Contains(container, kTestGood2));
  EXPECT_EQ(GetAmount(container, kTestGood1), 0);
  EXPECT_EQ(GetAmount(container, kTestGood2), 0);

  quantity += 1;
  container += quantity;
  EXPECT_EQ(GetAmount(container, kTestGood1), 1);
  EXPECT_EQ(GetAmount(container, quantity), 1);
  EXPECT_FALSE(Empty(container));

  Clear(&container);
  EXPECT_FALSE(Contains(container, kTestGood1));
  EXPECT_EQ(GetAmount(container, kTestGood1), 0);

  SetAmount(kTestGood1, 1, &container);
  EXPECT_EQ(GetAmount(container, kTestGood1), 1);

  quantity.set_amount(3);
  SetAmount(quantity, &container);
  EXPECT_EQ(GetAmount(container, kTestGood1), 3);

  Add(kTestGood1, 1, &container);
  EXPECT_EQ(GetAmount(container, kTestGood1), 4);

  Container container2;
  Move(kTestGood1, 1, &container, &container2);
  EXPECT_EQ(GetAmount(container, kTestGood1), 3);
  EXPECT_EQ(GetAmount(container2, kTestGood1), 1);

  Clear(&container);
  auto asVector = Expand(container);
  EXPECT_EQ(0, asVector.size());
  SetAmount(kTestGood1, 1, &container);
  asVector = Expand(container);
  EXPECT_EQ(1, asVector.size());
  EXPECT_EQ(1, asVector[0].amount());
  SetAmount(kTestGood2, 2, &container);
  asVector = Expand(container);
  EXPECT_EQ(2, asVector.size());
  EXPECT_EQ(kTestGood1, asVector[0].kind());
  EXPECT_EQ(kTestGood2, asVector[1].kind());
  EXPECT_EQ(1, asVector[0].amount());
  EXPECT_EQ(2, asVector[1].amount());
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
  EXPECT_EQ(GetAmount(container, kTestGood2), 1);
  EXPECT_EQ(quantity.amount(), 0);

  container >> quantity;
  EXPECT_EQ(GetAmount(container, kTestGood2), 0);
  EXPECT_EQ(quantity.amount(), 1);
}

TEST(GoodsUtilsTest, PlusAndMinus) {
  Quantity quantity;
  quantity.set_kind(kTestGood1);
  quantity += 10;
  EXPECT_EQ(10, quantity.amount());

  quantity -= 5;
  EXPECT_EQ(5, quantity.amount());

  Container container;
  container += quantity;
  container += quantity;
  EXPECT_EQ(10, GetAmount(container, quantity));
  container -= quantity;
  EXPECT_EQ(5, GetAmount(container, quantity));

  Quantity another;
  another.set_kind(kTestGood2);
  another += 10;
  container += another;
  EXPECT_EQ(5, GetAmount(container, quantity));
  EXPECT_EQ(another.amount(), GetAmount(container, another));

  Container barrel;
  barrel += container;
  EXPECT_EQ(GetAmount(container, kTestGood1), GetAmount(barrel, kTestGood1));
  EXPECT_EQ(GetAmount(container, kTestGood2), GetAmount(barrel, kTestGood2));

  Container box = barrel + container;
  EXPECT_EQ(GetAmount(container, kTestGood1) + GetAmount(barrel, kTestGood1),
            GetAmount(box, kTestGood1));
  EXPECT_EQ(GetAmount(container, kTestGood2) + GetAmount(barrel, kTestGood2),
            GetAmount(box, kTestGood2));

  box -= barrel;
  EXPECT_EQ(GetAmount(container, kTestGood1), GetAmount(box, kTestGood1));
  EXPECT_EQ(GetAmount(container, kTestGood2), GetAmount(box, kTestGood2));

  Container jar = barrel - container;
  EXPECT_EQ(GetAmount(container, kTestGood1) - GetAmount(barrel, kTestGood1),
            GetAmount(jar, kTestGood1));
  EXPECT_EQ(GetAmount(container, kTestGood2) - GetAmount(barrel, kTestGood2),
            GetAmount(jar, kTestGood2));
}

TEST(GoodsUtilsTest, Multiply) {
  Quantity quantity;
  quantity.set_kind(kTestGood1);
  quantity += 1;
  EXPECT_EQ(1.0, quantity.amount());
  quantity *= 2;
  EXPECT_EQ(2.0, quantity.amount());

  Container container;
  container += quantity;
  container *= 2;
  EXPECT_EQ(4.0, GetAmount(container, quantity));

  auto new_container = container * 2;
  EXPECT_EQ(8.0, GetAmount(new_container, quantity));

  auto new_quantity = quantity * 2;
  EXPECT_EQ(4.0, new_quantity.amount());
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
  EXPECT_EQ(GetAmount(container1, quantity1), GetAmount(container2, quantity1));
  EXPECT_EQ(GetAmount(container1, quantity2), GetAmount(container2, quantity2));
}

TEST(GoodsUtilsTest, CleanContainer) {
  Quantity quantity1;
  quantity1.set_kind(kTestGood1);

  Quantity quantity2;
  quantity2.set_kind(kTestGood2);
  quantity2 += 1;

  Container container;
  container << quantity1;
  container << quantity2;
  EXPECT_TRUE(Contains(container, quantity1));

  CleanContainer(&container);
  EXPECT_FALSE(Contains(container, quantity1));
  EXPECT_EQ(GetAmount(container, quantity2), 1);
}

TEST(GoodsUtilsTest, Iterator) {
  Quantity gold;
  gold.set_kind(kTestGood1);
  gold.set_amount(1);

  Container chest;
  chest << gold;
  EXPECT_EQ(gold.amount(), 0);
  EXPECT_EQ(GetAmount(chest, gold), 1.0);
  EXPECT_EQ(GetAmount(chest, kTestGood1), 1.0);

  for (const auto& quantity : chest.quantities()) {
    EXPECT_EQ(kTestGood1, quantity.first);
    EXPECT_EQ(1, quantity.second);
  }
}

TEST(GoodsUtilsTest, Relational) {
  Quantity gold;
  gold.set_kind(kTestGood1);
  gold.set_amount(10);
  Quantity salt;
  salt.set_kind(kTestGood2);
  salt.set_amount(10);

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
  gold += 5;
  bag << gold;
  EXPECT_TRUE(chest > bag);
  EXPECT_TRUE(chest >= bag);
  EXPECT_FALSE(chest < bag);
  EXPECT_FALSE(chest <= bag);

  EXPECT_FALSE(bag > chest);
  EXPECT_FALSE(bag >= chest);
  EXPECT_TRUE(bag < chest);
  EXPECT_TRUE(bag <= chest);

  salt += 15;
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

  gold += 7;
  salt += 12;
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
  EXPECT_EQ(2, chest * shaker);

  salt += 2;
  chest << salt;
  EXPECT_EQ(4, chest * shaker);

  salt += 2;
  shaker << salt;
  EXPECT_EQ(8, chest * shaker);
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

TEST(GoodsUtilsTest, SubtractFloor) {
  Quantity gold;
  gold.set_kind(kTestGood1);
  gold.set_amount(10);
  Quantity salt;
  salt.set_kind(kTestGood2);
  salt.set_amount(1);

  Container chest;
  chest << gold;
  Container thief;
  thief << salt;

  auto diff = SubtractFloor(chest, thief, 0);
  EXPECT_EQ(GetAmount(chest, gold), 10);
  EXPECT_EQ(GetAmount(chest, salt), 0);

  SetAmount(kTestGood1, -1000, &chest);
  SetAmount(kTestGood1, std::numeric_limits<int64>::max(), &thief);
  diff = SubtractFloor(chest, thief, -2000);
  EXPECT_EQ(-2000, GetAmount(diff, gold));

  SetAmount(kTestGood1, std::numeric_limits<int64>::max() - 1, &chest);
  SetAmount(kTestGood1, std::numeric_limits<int64>::min(), &thief);
  diff = SubtractFloor(chest, thief, 0);
  EXPECT_EQ(std::numeric_limits<int64>::max(), GetAmount(diff, gold));
}

TEST(GoodsUtilsTest, Copy) {
  Container source;
  Container target;
  Container mask;

  SetAmount(kTestGood1, 10, &source);
  SetAmount(kTestGood2, 10, &source);
  Copy(source, mask, &target);
  EXPECT_EQ(0, GetAmount(target, kTestGood1));
  EXPECT_EQ(0, GetAmount(target, kTestGood2));
  EXPECT_EQ(10, GetAmount(source, kTestGood1));
  EXPECT_EQ(10, GetAmount(source, kTestGood2));

  SetAmount(kTestGood1, 10, &mask);
  Copy(source, mask, &target);
  EXPECT_EQ(10, GetAmount(target, kTestGood1));
  EXPECT_EQ(0, GetAmount(target, kTestGood2));
  EXPECT_EQ(10, GetAmount(source, kTestGood1));
  EXPECT_EQ(10, GetAmount(source, kTestGood2));
  Copy(source, mask, &target);
  EXPECT_EQ(20, GetAmount(target, kTestGood1));
  EXPECT_EQ(0, GetAmount(target, kTestGood2));
  EXPECT_EQ(10, GetAmount(source, kTestGood1));
  EXPECT_EQ(10, GetAmount(source, kTestGood2));
}

TEST(GoodsUtilsTest, Erase) {
  Container source;
  Container target;

  source << kTestGood1;
  EXPECT_TRUE(Contains(source, kTestGood1));
  EXPECT_FALSE(Contains(source, kTestGood2));
  target << source;
  EXPECT_FALSE(Contains(source, kTestGood1));
  EXPECT_FALSE(Contains(source, kTestGood2));
  EXPECT_TRUE(Contains(target, kTestGood1));
  EXPECT_FALSE(Contains(target, kTestGood2));

  SetAmount(kTestGood1, 1, &source);
  SetAmount(kTestGood2, 0, &source);
  EXPECT_TRUE(Contains(source, kTestGood1));
  EXPECT_TRUE(Contains(source, kTestGood2));

  Erase(kTestGood1, &source);
  EXPECT_FALSE(Contains(source, kTestGood1));
  EXPECT_TRUE(Contains(source, kTestGood2));
  Erase(kTestGood2, &source);
  EXPECT_FALSE(Contains(source, kTestGood1));
  EXPECT_FALSE(Contains(source, kTestGood2));
}

TEST(GoodsUtilsTest, Iterators) {
  Container source;
  Container target;
  SetAmount(kTestGood1, 1, &source);
  SetAmount(kTestGood2, 2, &source);

  for (const auto& good : source.quantities()) {
    SetAmount(good, &target);
    EXPECT_EQ(GetAmount(target, good), good.second);
  }
  EXPECT_EQ(GetAmount(target, kTestGood1), GetAmount(source, kTestGood1));
  EXPECT_EQ(GetAmount(target, kTestGood2), GetAmount(source, kTestGood2));

  for (const auto& good : source.quantities()) {
    Erase(good, &target);
    EXPECT_EQ(0, GetAmount(target, good));
  }
  EXPECT_FALSE(Contains(target, kTestGood1));
  EXPECT_FALSE(Contains(target, kTestGood2));
}

TEST(GoodsUtilsTest, TradeGoods) {
  proto::TradeGood good;
  Container source;
  SetAmount(kTestGood1, 1, &source);
  CreateTradeGood(good);
  EXPECT_TRUE(ListGoods().empty());
  EXPECT_FALSE(AllGoodsExist(source));
  good.set_name(kTestGood1);
  CreateTradeGood(good);
  EXPECT_EQ(ListGoods().size(), 1);
  EXPECT_TRUE(Exists(good));
  EXPECT_TRUE(Exists(kTestGood1));
  EXPECT_TRUE(AllGoodsExist(source));
  SetAmount(kTestGood2, 1, &source);
  EXPECT_FALSE(AllGoodsExist(source));
}

TEST(GoodsUtilsTest, ScaleContainer) {
  proto::Container lhs;
  SetAmount(kTestGood1, 1, &lhs);
  SetAmount(kTestGood2, 10, &lhs);
  MultiplyU(lhs, micro::kTenInU);
  EXPECT_EQ(GetAmount(lhs, kTestGood1), 10);
  EXPECT_EQ(GetAmount(lhs, kTestGood2), 100);

  proto::Container lhs_u;
  SetAmount(kTestGood1, micro::kOneInU, &lhs_u);
  SetAmount(kTestGood2, micro::kTenInU, &lhs_u);
  MultiplyU(lhs_u, micro::kTenInU);
  EXPECT_EQ(GetAmount(lhs_u, kTestGood1), micro::kTenInU);
  EXPECT_EQ(GetAmount(lhs_u, kTestGood2), micro::kHundredInU);
}

TEST(GoodsUtilsTest, MatrixVectorMultiplication) {
  proto::Container rhs_u;
  SetAmount(kTestGood1, micro::kOneInU, &rhs_u);
  SetAmount(kTestGood2, micro::kTenInU, &rhs_u);

  proto::Container lhs;
  SetAmount(kTestGood1, 1, &lhs);
  SetAmount(kTestGood2, 10, &lhs);
  MultiplyU(lhs, rhs_u);
  EXPECT_EQ(GetAmount(lhs, kTestGood1), 1);
  EXPECT_EQ(GetAmount(lhs, kTestGood2), 100);

  proto::Container lhs_u;
  SetAmount(kTestGood1, micro::kOneInU, &lhs_u);
  SetAmount(kTestGood2, micro::kTenInU, &lhs_u);
  MultiplyU(lhs_u, rhs_u);
  EXPECT_EQ(GetAmount(lhs_u, kTestGood1), micro::kOneInU);
  EXPECT_EQ(GetAmount(lhs_u, kTestGood2), micro::kHundredInU);
}

} // namespace market
