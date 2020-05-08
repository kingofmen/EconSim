#include "util/status/status.h"

namespace util {

using google::protobuf::util::error::Code;

const Status& OkStatus() {
  static Status ok_status;
  return ok_status;
}

Status AlreadyExistsError(const std::string& msg) {
  return Status(Code::ALREADY_EXISTS, msg);
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

Status ResourceExhaustedError(const std::string& msg) {
  return Status(Code::RESOURCE_EXHAUSTED, msg);
}

bool Equal(const Status& one, const Status& two) {
  if (one.code() != two.code()) {
    return false;
  }
  if (one.error_message() != two.error_message()) {
    return false;
  }
  return true;
}

} // namespace util
