#include "util/status/status.h"

#include <ostream>

namespace util {

const Status& OkStatus() {
  static Status ok_status;
  return ok_status;
}

Status InvalidArgumentError(const std::string& msg) {
  return Status(Status::Code::INVALID_ARGUMENT, msg);
}

Status FailedPreconditionError(const std::string& msg) {
  return Status(Status::Code::FAILED_PRECONDITION, msg);
}

const std::string& Status::code_string(const Status::Code code) {
  static const std::string ok("OK");
  static const std::string invalid_arg("invalid argument");
  static const std::string failed_precondition("failed precondition");
  static const std::string unknown("unknown");
  switch (code) {
    case Status::Code::OK:
      return ok;
    case Status::Code::INVALID_ARGUMENT:
      return invalid_arg;
    case Status::Code::FAILED_PRECONDITION:
      return failed_precondition;
  }
  return unknown;
}

std::ostream& operator<<(std::ostream& os, const Status& status) {
  os << status.code_string(status.error_code());
  if (!status.ok()) {
    os << ": " << status.error_message();
  }
  return os;
}

} // namespace util
