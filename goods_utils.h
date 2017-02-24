// Utility functions for manipulating goods.

#include "proto/goods.pb.h"

namespace market {

// Returns true if con has an entry for the good name, even if the amount is zero.
bool Contains(const Container& con, const std::string name);
// Returns true if con has an entry for the good in qua. The amount is ignored.
bool Contains(const Container& con, const Quantity& qua);

// Returns the amount of name in con; zero if not set.
double GetAmount(const Container& con, const std::string name);
// Returns the amount of qua.kind() in con; zero if not set.
double GetAmount(const Container& con, const Quantity& qua);

// Create an entry for the good name in con. No effect if the entry exists already.
Container& operator<<(Container& con, std::string name);

Quantity &operator+=(Quantity &lhs, const double rhs);
Quantity &operator-=(Quantity &lhs, const double rhs);

Container &operator+=(Container &lhs, const Container &rhs);
Container &operator+=(Container &lhs, const Quantity &rhs);
Container &operator-=(Container &lhs, const Container &rhs);
Container &operator-=(Container &lhs, const Quantity &rhs);
Container operator+(Container lhs, const Container &rhs);
Container operator-(Container lhs, const Container &rhs);

} // namespace market
