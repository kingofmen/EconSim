// Utility functions for manipulating goods.

#include "proto/goods.pb.h"

namespace market {

// Adds the amount in dis to dat.
void combine(const Quantity &dis, Quantity *dat);

Quantity &operator+=(Quantity &lhs, const double rhs);
Quantity &operator-=(Quantity &lhs, const double rhs);

Container &operator+=(Container &lhs, const Container &rhs);
Container &operator+=(Container &lhs, const Quantity &rhs);
Container &operator-=(Container &lhs, const Container &rhs);
Container &operator-=(Container &lhs, const Quantity &rhs);
Container operator+(Container lhs, const Container &rhs);
Container operator-(Container lhs, const Container &rhs);

} // namespace market
