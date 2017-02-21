// Utility functions for goods protos.
#include "goods_utils.h"

namespace market {

// Adds dis into dat. Does not check for equality of goods, nor empty dis.
void combine(const Quantity& dis, Quantity* dat) {
  dat->set_amount(dis.amount() + dat->amount());
}

} // namespace market
