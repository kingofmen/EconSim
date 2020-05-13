// Class representing connections between areas.
#ifndef BASE_GEOGRAPHY_CONNECTION_H
#define BASE_GEOGRAPHY_CONNECTION_H

#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "games/geography/geography.h"
#include "games/geography/mobile.h"
#include "games/geography/proto/geography.pb.h"
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
  Area* a() { return Area::GetById(proto_.a_area_id()); }
  Area* z() { return Area::GetById(proto_.z_area_id()); }
  const Area* a() const { return Area::GetById(proto_.a_area_id()); }
  const Area* z() const { return Area::GetById(proto_.z_area_id()); }
  const util::proto::ObjectId& a_id() const { return proto_.a_area_id(); }
  const util::proto::ObjectId& z_id() const { return proto_.z_area_id(); }

  // ID of the other side.
  const util::proto::ObjectId& OtherSide(const util::proto::ObjectId& id) const;

  // Other-side area.
  Area* OtherSide(const Area* area);
  const Area* OtherSide(const Area* area) const;

  uint64 connection_id() const { return proto_.id();}
  // Deprecated, used connection_id instead.
  uint64 ID() const { return proto_.id(); }
  uint64 length_u() const { return proto_.distance_u(); }
  uint64 width_u() const { return proto_.width_u(); }
  geography::proto::ConnectionType type() const { return proto_.type(); }

  // Proto access.
  const proto::Connection& Proto() const { return proto_; }
  proto::Connection* mutable_proto() { return &proto_; }

  // Builder method.
  static std::unique_ptr<Connection> FromProto(const proto::Connection& conn);

  // Lookup by endpoint; both A and Z connections are returned.
  static const std::unordered_set<Connection*>&
  ByEndpoint(const util::proto::ObjectId& id);

  // Lookup by both endpoints.
  static const std::unordered_set<Connection*>&
  ByEndpoints(const util::proto::ObjectId& a, const util::proto::ObjectId& z);

  // Lookup by connection ID.
  static Connection* ById(uint64 conn_id);

private:
  Connection() = delete;
  Connection(const proto::Connection& conn);

  // The underlying data.
  proto::Connection proto_;

  // Listener map.
  std::unordered_map<util::proto::ObjectId, Listener> listeners_;
};

} // namespace geography

#endif
