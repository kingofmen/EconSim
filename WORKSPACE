# Diff: &"C:\Program Files\Git\usr\bin\diff.exe" --ignore-all-space file1 file2
# Needed to build protobuf, see https://github.com/protocolbuffers/protobuf/issues/5051.
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Protobuf dependencies that it doesn't load for itself, probably because of the indirection.
# Copied from https://github.com/protocolbuffers/protobuf/blob/main/protobuf_deps.bzl.
http_archive(
    name = "bazel_skylib",
    sha256 = "97e70364e9249702246c0e9444bccdc4b847bed1eb03c5a3ece4f83dfe6abc44",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.0.2/bazel-skylib-1.0.2.tar.gz",
        "https://github.com/bazelbuild/bazel-skylib/releases/download/1.0.2/bazel-skylib-1.0.2.tar.gz",
    ],
)
http_archive(
    name = "rules_pkg",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_pkg/releases/download/0.5.1/rules_pkg-0.5.1.tar.gz",
        "https://github.com/bazelbuild/rules_pkg/releases/download/0.5.1/rules_pkg-0.5.1.tar.gz",
    ],
    sha256 = "a89e203d3cf264e564fcb96b6e06dd70bc0557356eb48400ce4b5d97c2c3720d",
)
http_archive(
    name = "rules_python",
    sha256 = "b6d46438523a3ec0f3cead544190ee13223a52f6a6765a29eae7b7cc24cc83a0",
    urls = ["https://github.com/bazelbuild/rules_python/releases/download/0.1.0/rules_python-0.1.0.tar.gz"],
)

# Golang rules.
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "io_bazel_rules_go",
    sha256 = "8e968b5fcea1d2d64071872b12737bbb5514524ee5f0a4f54f5920266c261acb",
    url = "https://github.com/bazelbuild/rules_go/releases/download/v0.28.0/rules_go-v0.28.0.zip",
)

load("@io_bazel_rules_go//go:deps.bzl", "go_register_toolchains", "go_rules_dependencies")

go_rules_dependencies()
go_register_toolchains(version = "1.16.5")

# Needed for Abseil.
http_archive(
    name = "rules_cc",
    strip_prefix = "rules_cc-main",
    urls = ["https://github.com/bazelbuild/rules_cc/archive/master.zip"],
)

# Needed to let proto_library rules find protoc.
# Known issue: https://github.com/google/protobuf/issues/3766
local_repository(
    name = "com_google_protobuf",
    path = "C:\\Users\\Rolf\\base\\protobuf\\",
)

local_repository(
    name = "com_google_protobuf_cc",
    path = "C:\\Users\\Rolf\\base\\protobuf\\",
)

new_local_repository(
    name = "gtest",
    path = "C:\\Users\\Rolf\\base\\protobuf\\",
    build_file = "gmock.BUILD",
)

new_local_repository(
    name = "freetype",
    path = "C:\\Users\\Rolf\\base\\third_party\\freetype\\",
    build_file = "C:\\Users\\Rolf\\base\\third_party\\freetype\\BUILD",
)

new_local_repository(
    name = "sdl_ttf",
    path = "C:\\Users\\Rolf\\base\\third_party\\SDL_TTF\\",
    build_file = "C:\\Users\\Rolf\\base\\third_party\\SDL_TTF\\BUILD",
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

new_local_repository(
    name = "sdl2",
    path = "C:\\Users\\Rolf\\base\\third_party\\sdl2",
    build_file = "C:\\Users\\Rolf\\base\\third_party\\sdl2\\BUILD.bazel",
)
