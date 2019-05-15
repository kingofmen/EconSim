#include "util/proto/file.h"

#include <fstream>

#include "absl/strings/substitute.h"
#include "google/protobuf/text_format.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "util/status/status.h"

namespace util {
namespace proto {

google::protobuf::util::Status
ParseProtoFile(const std::string& filename, google::protobuf::Message* proto) {
  std::ifstream reader(filename);
  if (!reader.good()) {
    return util::InvalidArgumentError("Could not open file");
  }
  google::protobuf::io::IstreamInputStream input(&reader);
  if (!google::protobuf::TextFormat::Parse(&input, proto)) {
    return util::InvalidArgumentError(absl::Substitute("Error parsing file $0", filename));
  }

  reader.close();
  return util::OkStatus();
}

google::protobuf::util::Status
MergeProtoFile(const std::string& filename, google::protobuf::Message* proto) {
  std::ifstream reader(filename);
  if (!reader.good()) {
    return util::InvalidArgumentError("Could not open file");
  }
  google::protobuf::io::IstreamInputStream input(&reader);
  if (!google::protobuf::TextFormat::Merge(&input, proto)) {
    return util::InvalidArgumentError(absl::Substitute("Error parsing file $0", filename));
  }

  reader.close();
  return util::OkStatus();
}

}  // namespace proto
}  // namespace util
