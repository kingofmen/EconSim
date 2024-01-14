load("@io_bazel_rules_go//go:def.bzl", "go_binary", "go_library")

go_binary(
    name = "ebit_client",
    embed = [":ebiten_lib"],
    importpath = "gogames/settlers/clients/ebiten",
    visibility = ["//visibility:public"],
)

go_library(
    name = "ebiten_lib",
    srcs = ["main.go"],
    importpath = "gogames/settlers/clients/ebiten",
    visibility = ["//visibility:private"],
    deps = [
        "//gogames/settlers/engine:settlers",
      	"//gogames/tiles/triangles:triangles",
        "@com_github_hajimehoshi_ebiten_v2//ebitenutil:go_default_library",
        "@com_github_hajimehoshi_ebiten_v2//:go_default_library",
    ],
)
