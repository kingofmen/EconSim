#include "util/status/status.h"

namespace util {

using google::protobuf::util::error::Code;

const Status& OkStatus() {
  static Status ok_status;
  return ok_status;
}

Status InvalidArgumentError(const std::string& msg) {
  return Status(Code::INVALID_ARGUMENT, msg);
}

Status FailedPreconditionError(const std::string& msg) {
  return Status(Code::FAILED_PRECONDITION, msg);
}

Status NotFoundError(const std::string& msg) {
  return Status(Code::NOT_FOUND, msg);
}

Status NotImplementedError(const std::string& msg) {
  return Status(Code::UNIMPLEMENTED, msg);
}

} // namespace util
