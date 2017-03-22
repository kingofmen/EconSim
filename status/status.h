// Status object for returning error messages.

#ifndef UTIL_STATUS_H
#define UTIL_STATUS_H

#include <string>
//#include <string_view>

namespace util {

class Status {
public:
  enum Code {
    OK = 0,
    INVALID_ARGUMENT = 1,
  };

  Status() : code_(OK) {}
  // TODO: Use string_view when I have updated the compiler.
  //Status(Code code, std::string_view msg) : code_(code), message_(msg) {}
  Status(Code code, const std::string& msg) : code_(code), message_(msg) {}
  Status(const Status &other) : code_(other.code_), message_(other.message_) {}

  bool ok() const { return code_ == OK; }
  const std::string &error_message() const { return message_; }

private:
  Code code_;
  std::string message_;
};

const Status& OkStatus();

Status InvalidArgumentError(const std::string& msg);

} // namespace util

#endif // UTIL_STATUS_H
