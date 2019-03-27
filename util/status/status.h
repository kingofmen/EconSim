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
    FAILED_PRECONDITION = 2,
  };

  Status() : code_(OK) {}
  // TODO: Use string_view when I have updated the compiler.
  //Status(Code code, std::string_view msg) : code_(code), message_(msg) {}
  Status(Code code, const std::string& msg) : code_(code), message_(msg) {}
  Status(const Status &other) : code_(other.code_), message_(other.message_) {}

  bool ok() const { return code_ == OK; }
  const std::string &error_message() const { return message_; }
  Code error_code() const {return code_;}
  static const std::string& code_string(const Code code);
  
private:
  Code code_;
  std::string message_;
};

const Status& OkStatus();

Status InvalidArgumentError(const std::string& msg);
Status FailedPreconditionError(const std::string& msg);

std::ostream& operator<<(std::ostream& os, const Status& status);
} // namespace util

#endif // UTIL_STATUS_H
