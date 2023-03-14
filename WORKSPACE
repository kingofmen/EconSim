# Diff: &"C:\Program Files\Git\usr\bin\diff.exe" --ignore-all-space file1 file2
# Needed to build protobuf, see https://github.com/protocolbuffers/protobuf/issues/5051.
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Protobuf dependencies that it doesn't load for itself, probably because of the indirection.
# Copied from https://github.com/protocolbuffers/protobuf/blob/main/protobuf_deps.bzl.
#http_archive(
#    name = "bazel_skylib",
#    sha256 = "97e70364e9249702246c0e9444bccdc4b847bed1eb03c5a3ece4f83dfe6abc44",
#    urls = [
#        "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.0.2/bazel-skylib-1.0.2.tar.gz",
#        "https://github.com/bazelbuild/bazel-skylib/releases/download/1.0.2/bazel-skylib-1.0.2.tar.gz",
#    ],
#)
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
http_archive(
    name = "io_bazel_rules_go",
    sha256 = "56d8c5a5c91e1af73eca71a6fab2ced959b67c86d12ba37feedb0a2dfea441a6",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_go/releases/download/v0.37.0/rules_go-v0.37.0.zip",
        "https://github.com/bazelbuild/rules_go/releases/download/v0.37.0/rules_go-v0.37.0.zip",
    ],
)

load("@io_bazel_rules_go//go:deps.bzl", "go_rules_dependencies", "go_register_toolchains")

go_rules_dependencies()
go_register_toolchains(version = "1.18.10")

# Needed for Abseil.
http_archive(
    name = "rules_cc",
    strip_prefix = "rules_cc-main",
    urls = ["https://github.com/bazelbuild/rules_cc/archive/master.zip"],
)

# Protobuf support.
http_archive(
    name = "com_google_protobuf",
    sha256 = "d0f5f605d0d656007ce6c8b5a82df3037e1d8fe8b121ed42e536f569dec16113",
    strip_prefix = "protobuf-3.14.0",
    urls = [
        "https://mirror.bazel.build/github.com/protocolbuffers/protobuf/archive/v3.14.0.tar.gz",
        "https://github.com/protocolbuffers/protobuf/archive/v3.14.0.tar.gz",
    ],
)
load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")
protobuf_deps()

new_local_repository(
    name = "gtest",
    path = "C:\\users\\rolfa\\base\\protobuf\\",
    build_file = "gmock.BUILD",
)

new_local_repository(
    name = "freetype",
    path = "C:\\users\\rolfa\\base\\third_party\\freetype\\",
    build_file = "C:\\users\\rolfa\\base\\third_party\\freetype\\BUILD",
)

new_local_repository(
    name = "sdl_ttf",
    path = "C:\\users\\rolfa\\base\\third_party\\SDL_TTF\\",
    build_file = "C:\\users\\rolfa\\base\\third_party\\SDL_TTF\\BUILD",
)

local_repository(
    name = "com_google_absl",
    path = "C:\\users\\rolfa\\base\\third_party\\abseil-cpp",
)

new_local_repository(
    name = "horde3d",
    path = "C:\\users\\rolfa\\base\\third_party\\Horde3D",
    build_file = "C:\\users\\rolfa\\base\\third_party\\Horde3D\\horde3d.BUILD",
)

new_local_repository(
    name = "sdl2",
    path = "C:\\users\\rolfa\\base\\third_party\\sdl2",
    build_file = "C:\\users\\rolfa\\base\\third_party\\sdl2\\BUILD.bazel",
)
