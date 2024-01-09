# Diff: &"C:\Program Files\Git\usr\bin\diff.exe" --ignore-all-space file1 file2
# Needed for Internet repos.
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

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

# Gazelle for go_repository support.
http_archive(
    name = "bazel_gazelle",
    sha256 = "ecba0f04f96b4960a5b250c8e8eeec42281035970aa8852dda73098274d14a1d",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-gazelle/releases/download/v0.29.0/bazel-gazelle-v0.29.0.tar.gz",
        "https://github.com/bazelbuild/bazel-gazelle/releases/download/v0.29.0/bazel-gazelle-v0.29.0.tar.gz",
    ],
)

load("@io_bazel_rules_go//go:deps.bzl", "go_rules_dependencies", "go_register_toolchains")
load("@bazel_gazelle//:deps.bzl", "gazelle_dependencies", "go_repository")

go_rules_dependencies()
go_register_toolchains(version = "1.18.10")
gazelle_dependencies()

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

go_repository(
    name = "com_github_google_go_cmp",
    importpath = "github.com/google/go-cmp",
    sum = "h1:pJfrTSHC+QpCQplFZqzlwihfc+0Oty0ViHPHPxXj0SI=",
    version = "v0.5.3-0.20201020212313-ab46b8bd0abd",
)

# Note that 'vcs' is needed for 'v2' suffix to work. And 'remote'
# is required for 'vcs'.
go_repository(
    name = "com_github_hajimehoshi_ebiten_v2",
    commit = "fdf36026aee97e674f23c219f7cfc2a544b13f51",
    importpath = "github.com/hajimehoshi/ebiten/v2",
    remote = "https://github.com/hajimehoshi/ebiten",
    vcs = "git",
)

# Needed for ebiten.
go_repository(
    name = "com_github_ebitengine_purego",
    importpath = "github.com/ebitengine/purego",
    commit = "dab77e60781e3ee1254ff2f5025c9e369c21100e",
)

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
