// Class to represent geographic locations.
#ifndef BASE_GEOGRAPHY_GEOGRAPHY_H
#define BASE_GEOGRAPHY_GEOGRAPHY_H

#include <unordered_map>

#include "geography/proto/geography.pb.h"
#include "google/protobuf/stubs/status.h"
#include "industry/industry.h"
#include "market/market.h"
#include "market/proto/market.pb.h"

namespace geography {

// Filters which return true if this Field has the requirements for at least
// one variant in each step of the given production chain.
bool HasFixedCapital(const proto::Field& field,
                     const industry::Production& production);
bool HasLandType(const proto::Field& field,
                 const industry::Production& production);
bool HasRawMaterials(const proto::Field& field,
                     const industry::Production& production);

// Returns a production chain to transform the field into a different land type.
google::protobuf::util::Status
GenerateTransitionProcess(const proto::Field& field,
                          const proto::Transition& transition,
                          industry::Production* production);

// Class representing a collection of economic activity close enough together
// that internal logistics can reasonably be abstracted over.
class Area {
public:
  ~Area();

  const market::proto::Container& GetPricesU() const {
    return market_.Proto().prices_u();
  }
  void Update();

  uint64 id() const { return proto_.id(); }

  proto::Area* Proto() { return &proto_; }
  market::Market* mutable_market() { return &market_; }

  static Area* GetById(uint64 id);
  static std::unique_ptr<Area> FromProto(const proto::Area& area);

private:
  Area() = delete;
  Area(const proto::Area& area);
  proto::Area proto_;
  market::Market market_;

  static std::unordered_map<uint64, Area*> id_map_;
};

} // namespace geography

#endif
