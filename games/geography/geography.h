// Class to represent geographic locations.
#ifndef BASE_GEOGRAPHY_GEOGRAPHY_H
#define BASE_GEOGRAPHY_GEOGRAPHY_H

#include <unordered_map>

#include "games/geography/proto/geography.pb.h"
#include "games/industry/industry.h"
#include "games/market/market.h"
#include "games/market/proto/market.pb.h"
#include "util/status/status.h"

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
util::Status GenerateTransitionProcess(const proto::Field& field,
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

  const util::proto::ObjectId& area_id() const;

  proto::Area* Proto() { return &proto_; }
  const proto::Area* Proto() const { return &proto_; }
  market::Market* mutable_market() { return &market_; }
  const market::Market& market() const { return market_; }
  int numPops() const { return proto_.pop_ids_size(); }
  const std::vector<uint64> pop_ids() const;
  int num_fields() const { return proto_.fields_size(); }
  const std::vector<const proto::Field*> fields() const;
  const proto::Field* field(int idx) const { return &proto_.fields(idx); }
  proto::Field* mutable_field(int idx) { return proto_.mutable_fields(idx); }

  static Area* GetById(const util::proto::ObjectId& id);
  static std::unique_ptr<Area> FromProto(const proto::Area& area);

private:
  Area() = delete;
  Area(const proto::Area& area);
  proto::Area proto_;
  market::Market market_;
};

Area* ById(const util::proto::ObjectId& area_id);

} // namespace geography

#endif
