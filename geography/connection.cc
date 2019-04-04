#include "geography/connection.h"

#include <memory>

namespace geography {

std::unique_ptr<Connection>
Connection::FromProto(const proto::Connection& conn) {
  std::unique_ptr<Connection> ret;
  // TODO: Actually handle these errors.
  if (conn.a() == 0) {
    return ret;
  }
  if (conn.z() == 0) {
    return ret;
  }
  if (conn.distance() == 0) {
    return ret;
  }
  if (conn.width() == 0) {
    return ret;
  }

  ret.reset(new Connection(conn));
  return ret;
}

Connection::Connection(const proto::Connection& conn) : proto_(conn) {}

} // namespace geography
