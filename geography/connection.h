// Class representing connections between areas.
#ifndef BASE_GEOGRAPHY_CONNECTION_H
#define BASE_GEOGRAPHY_CONNECTION_H

#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "geography/geography.h"
#include "geography/mobile.h"
#include "geography/proto/geography.pb.h"
#include "util/proto/object_id.h"
#include "util/headers/int_types.h"

namespace geography {

// Bidirectional connections between Areas. Note that endpoints are denoted 'a'
// and 'z', but this is only for convenience - Connections may be looked up by
// either endpoint, and are fully symmetric. Note that there may be more than
// one Connection between two Areas.
class Connection {
public:
  ~Connection();

  struct Detection {
    util::proto::ObjectId unit_id;
    int64 see_target;
    int64 target_sees;
  };
  typedef std::function<Detection(const Mobile&)> Listener;

  // Callbacks for detection and evasion.
  void Register(const util::proto::ObjectId& unit_id, Listener l);
  void UnRegister(const util::proto::ObjectId& unit_id);
  void Listen(const Mobile& mobile, uint64 distance_u,
              std::vector<Detection>* detections) const;

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

  uint64 ID() const { return proto_.id(); }
  uint64 length_u() const { return proto_.distance_u(); }
  uint64 width_u() const { return proto_.width_u(); }
  geography::proto::ConnectionType type() const { return proto_.type(); }

  // Proto access.
  const proto::Connection& Proto() const { return proto_; }

  // Builder method.
  static std::unique_ptr<Connection> FromProto(const proto::Connection& conn);

  // Lookup by endpoint; both A and Z connections are returned.
  static const std::unordered_set<Connection*>& ByEndpoint(uint64 area_id);

  // Lookup by both endpoints.
  static const std::unordered_set<Connection*>& ByEndpoints(uint64 area_one,
                                                            uint64 area_two);

  // Lookup by connection ID.
  static Connection* ById(uint64 conn_id);

private:
  Connection() = delete;
  Connection(const proto::Connection& conn);

  // The underlying data.
  proto::Connection proto_;

  // Listener map.
  std::unordered_map<util::proto::ObjectId, Listener> listeners_;

  // ID mapping.
  static std::unordered_map<uint64, Connection*> id_map_;

  // Endpoint mapping.
  static std::unordered_map<uint64, std::unordered_set<Connection*>>
      endpoint_map_;

  // Mapping by both endpoints.
  static std::unordered_map<uint64, std::unordered_set<Connection*>>
      both_endpoints_map_;
};

// Interface for moving Mobiles.
class Traverser {
public:
  // Updates location; returns true if final destination is reached.
  virtual bool Traverse(const Mobile& mobile,
                        geography::proto::Location* location) const = 0;
};

// Default implementation.
class DefaultTraverser : public Traverser {
public:
  bool Traverse(const Mobile& mobile,
                geography::proto::Location* location) const override;
};

} // namespace geography

#endif
