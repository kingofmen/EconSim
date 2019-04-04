# Needed to build protobuf, see https://github.com/protocolbuffers/protobuf/issues/5051.
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
http_archive(
    name = "bazel_skylib",
    sha256 = "bbccf674aa441c266df9894182d80de104cabd19be98be002f6d478aaa31574d",
    strip_prefix = "bazel-skylib-2169ae1c374aab4a09aa90e65efe1a3aad4e279b",
    urls = ["https://github.com/bazelbuild/bazel-skylib/archive/2169ae1c374aab4a09aa90e65efe1a3aad4e279b.tar.gz"],
)

# Needed to let proto_library rules find protoc.
# Known issue: https://github.com/google/protobuf/issues/3766
local_repository(
  name = "com_google_protobuf",
  path = "C:\\Users\\Rolf\\base\\protobuf\\"
)

local_repository(
  name = "com_google_protobuf_cc",
  path = "C:\\Users\\Rolf\\base\\protobuf\\"
)

new_local_repository(
  name = "gtest",
  path = "C:\\Users\\Rolf\\base\\protobuf\\",
  build_file = "gmock.BUILD",
)

local_repository(
  name = "com_google_absl",
  path = "C:\\Users\\Rolf\\base\\third_party\\abseil-cpp",
)

new_local_repository(
  name = "ogre",
  path = "C:\\Users\\Rolf\\base\\third_party\\ogre",
  build_file = "C:\\Users\\Rolf\\base\\third_party\\ogre\\ogre.BUILD",
)

new_local_repository(
  name = "horde3d",
  path = "C:\\Users\\Rolf\\base\\third_party\\Horde3D",
  build_file = "C:\\Users\\Rolf\\base\\third_party\\Horde3D\\horde3d.BUILD",
)


