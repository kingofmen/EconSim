// Utility functions for goods protos.
#include "goods_utils.h"

namespace market {

// Adds dis into dat. Does not check for equality of goods, nor empty dis.
void combine(const Quantity &dis, Quantity *dat) {
  dat->set_amount(dis.amount() + dat->amount());
}

Quantity &operator+=(Quantity &lhs, const double rhs) {
  lhs.set_amount(lhs.amount() + rhs);
  return lhs;
}

Quantity &operator-=(Quantity &lhs, const double rhs) {
  lhs.set_amount(lhs.amount() - rhs);
  return lhs;
}

Container &operator+=(Container &lhs, const Container &rhs) {
  auto &quantities = *lhs.mutable_quantities();
  for (const auto &rhs_quantity : rhs.quantities()) {
    auto &lhs_quantity = quantities[rhs_quantity.first];
    lhs_quantity += rhs_quantity.second.amount();
  }
  return lhs;
}

Container &operator+=(Container &lhs, const Quantity &rhs) {
  auto &quantities = *lhs.mutable_quantities();
  quantities[rhs.kind()] += rhs.amount();
  return lhs;
}

Container &operator-=(Container &lhs, const Container &rhs) {
  auto &quantities = *lhs.mutable_quantities();
  for (const auto &rhs_quantity : rhs.quantities()) {
    auto &lhs_quantity = quantities[rhs_quantity.first];
    lhs_quantity -= rhs_quantity.second.amount();
  }
  return lhs;
}

Container &operator-=(Container &lhs, const Quantity &rhs) {
  auto &quantities = *lhs.mutable_quantities();
  quantities[rhs.kind()] -= rhs.amount();
  return lhs;
}

Container operator+(Container lhs, const Container &rhs) {
  lhs += rhs;
  return lhs;
}

Container operator-(Container lhs, const Container &rhs) {
  lhs -= rhs;
  return lhs;
}

} // namespace market
