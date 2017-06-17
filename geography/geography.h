// Class to represent geographic locations.
#ifndef BASE_GEOGRAPHY_GEOGRAPHY_H
#define BASE_GEOGRAPHY_GEOGRAPHY_H

#include "geography/proto/geography.pb.h"
#include "industry/proto/industry.pb.h"
#include "market/market.h"
#include "market/proto/market.pb.h"
#include "util/status/status.h"

#include <iostream>

namespace geography {

// Filters which return true if this Field has the requirements for at least
// one variant in each step of the given production chain.
bool HasFixedCapital(const proto::Field& field,
                     const industry::proto::Production& production);
bool HasLandType(const proto::Field& field,
                 const industry::proto::Production& production);
bool HasRawMaterials(const proto::Field& field,
                     const industry::proto::Production& production);

// Returns a production chain to transform the field into a different land type.
util::Status GenerateTransitionProcess(const proto::Field& field,
                                       const proto::Transition& transition,
                                       industry::proto::Production* production);

class Area {
public:
  Area() = default;
  Area(const market::proto::MarketProto& market) : market_(market) {}
  Area(const proto::Area& area) : proto_(area) {
  }

  const market::proto::Container& GetPrices() const { return market_.prices(); }
  void Update();

  proto::Area* Proto() {return &proto_;}
  market::Market* mutable_market() { return &market_; }

private:
  proto::Area proto_;
  market::Market market_;
};

} // namespace geography

#endif
