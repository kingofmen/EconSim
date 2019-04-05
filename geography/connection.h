// Class representing connections between areas.
#ifndef BASE_GEOGRAPHY_CONNECTION_H
#define BASE_GEOGRAPHY_CONNECTION_H

#include <memory>

#include "geography/geography.h"
#include "geography/proto/geography.pb.h"

namespace geography {

class Connection {
 public:
  static std::unique_ptr<Connection> FromProto(const proto::Connection& conn);

  Area* a() { return Area::GetById(proto_.a()); }
  Area* z() { return Area::GetById(proto_.z()); }

private:
  Connection() = delete;
  Connection(const proto::Connection& conn);

  // The underlying data.
  proto::Connection proto_;
};


} // namespace geography


#endif
