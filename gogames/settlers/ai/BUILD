load("@io_bazel_rules_go//go:def.bzl", "go_library")

go_library(
    name = "cog",
    srcs = ["random.go"],
    importpath = "gogames/settlers/ai/cog",
    visibility = ["//visibility:public"],
    deps = [
        "//gogames/settlers/engine:settlers",
      	"//gogames/tiles/triangles:triangles",
    ],
)
