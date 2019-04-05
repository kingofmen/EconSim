// Class representing connections between areas.
#ifndef BASE_GEOGRAPHY_CONNECTION_H
#define BASE_GEOGRAPHY_CONNECTION_H

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "geography/geography.h"
#include "geography/proto/geography.pb.h"

namespace geography {

// Bidirectional connections between Areas. Note that endpoints are denoted 'a'
// and 'z', but this is only for convenience - Connections may be looked up by
// either endpoint.
class Connection {
 public:

  // Endpoint access.
  Area* a() { return Area::GetById(proto_.a()); }
  Area* z() { return Area::GetById(proto_.z()); }
  const Area* a() const { return Area::GetById(proto_.a()); }
  const Area* z() const { return Area::GetById(proto_.z()); }

  // ID of the other side.
  uint64 OtherSide(uint64 area_id) const;

  // Other-side area.
  Area* OtherSide(const Area* area);
  const Area* OtherSide(const Area* area) const;

  // Builder method.
  static std::unique_ptr<Connection> FromProto(const proto::Connection& conn);

  // Lookup by endpoint; both A and Z connections are returned.
  static const std::unordered_set<Connection*>& ByEndpoint(uint64 area_id);

private:
  Connection() = delete;
  Connection(const proto::Connection& conn);

  // The underlying data.
  proto::Connection proto_;

  // Endpoint mapping.
  static std::unordered_map<uint64, std::unordered_set<Connection*>>
      endpoint_map_;
};


} // namespace geography


#endif
