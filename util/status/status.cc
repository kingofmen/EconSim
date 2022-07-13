#include "util/status/status.h"

namespace util {

using absl::StatusCode;

const Status& OkStatus() {
  static Status ok_status;
  return ok_status;
}

Status AlreadyExistsError(const std::string& msg) {
  return Status(StatusCode::kAlreadyExists, msg);
}

Status InvalidArgumentError(const std::string& msg) {
  return Status(StatusCode::kInvalidArgument, msg);
}

Status FailedPreconditionError(const std::string& msg) {
  return Status(StatusCode::kFailedPrecondition, msg);
}

Status NotFoundError(const std::string& msg) {
  return Status(StatusCode::kNotFound, msg);
}

Status NotImplementedError(const std::string& msg) {
  return Status(StatusCode::kUnimplemented, msg);
}

Status ResourceExhaustedError(const std::string& msg) {
  return Status(StatusCode::kResourceExhausted, msg);
}

const Status& NotComplete() {
  static Status not_complete(StatusCode::kResourceExhausted, "not completed");
  return not_complete;
}

bool Equal(const Status& one, const Status& two) {
  if (one.code() != two.code()) {
    return false;
  }
  if (one.message() != two.message()) {
    return false;
  }
  return true;
}

bool IsNotComplete(const Status& cand) {
  return Equal(cand, NotComplete());
}

} // namespace util
