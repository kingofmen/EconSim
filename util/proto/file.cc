#include "util/proto/file.h"

#include <fstream>

#include "google/protobuf/text_format.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"

namespace util {
namespace proto {

util::Status ParseProtoFile(const std::string& filename,
                            google::protobuf::Message* proto) {
  std::ifstream reader(filename);
  if (!reader.good()) {
    return util::InvalidArgumentError("Could not open file");
  }
  google::protobuf::io::IstreamInputStream input(&reader);
  if (!google::protobuf::TextFormat::Parse(&input, proto)) {
    return util::InvalidArgumentError("Error parsing file");
  }

  reader.close();
  return util::OkStatus();
}

}  // namespace proto
}  // namespace util
