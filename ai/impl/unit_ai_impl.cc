#include "unit_ai_impl.h"

namespace ai {
namespace impl {

void ShuttleTrader::AddStepsToPlan(const units::Unit& unit,
                                   actions::proto::Strategy* strategy,
                                   actions::proto::Plan* plan) const {
  if (!strategy->has_shuttle_trade()) {
    // TODO: Handle this as error? It seems to indicate something unexpected
    // anyway.
    return;
  }
}

} // namespace impl
} // namespace ai
