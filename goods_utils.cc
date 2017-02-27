// Utility functions for goods protos.
#include "goods_utils.h"

namespace market {

bool Contains(const Container& con, const std::string name) {
  return con.quantities().find(name) != con.quantities().end();
}

bool Contains(const Container& con, const Quantity& qua) {
  return Contains(con, qua.kind());
}

double GetAmount(const Container& con, const std::string name) {
  if (!Contains(con, name)) {
    return 0;
  }
  return con.quantities().at(name).amount();
}

double GetAmount(const Container& con, const Quantity& qua) {
  return GetAmount(con, qua.kind());
}

void Clear(Container& con) {
  auto& quantities = *con.mutable_quantities();
  for (auto& quantity : quantities) {
    quantity.second.set_amount(0);
  }
}

Container& operator<<(Container& con, Quantity& qua) {
  con += qua;
  qua.set_amount(0);
  return con;
}

Container& operator>>(Container& con, Quantity& qua) {
  qua.set_amount(qua.amount() + GetAmount(con, qua.kind()));
  auto& quantities = *con.mutable_quantities();
  quantities[qua.kind()].set_amount(0);
  return con;
}

Container& operator<<(Container& con, std::string name) {
  if (Contains(con, name)) {
    return con;
  }
  auto& quantities = *con.mutable_quantities();
  quantities[name].set_amount(0);
  return con;
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
