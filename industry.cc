// Implementation of production chains.
#include "industry.h"

namespace industry {
using market::proto::Container;

void Production::PerformStep(Container *inputs, Container *outputs) {
  ++current_step_;
}

bool Production::Complete() const {
  return current_step_ >= steps_size();
}

} // namespace industry
