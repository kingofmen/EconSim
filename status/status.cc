#include "util/status/status.h"

namespace util {

const Status& OkStatus() {
  static Status ok_status;
  return ok_status;
}

Status InvalidArgumentError(const std::string& msg) {
  return Status(Status::Code::INVALID_ARGUMENT, msg);
}

} // namespace util
