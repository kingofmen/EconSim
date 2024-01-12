load("@io_bazel_rules_go//go:def.bzl", "go_library", "go_test")

go_library(
    name = "triangles",
    srcs = [
      "triangles.go",
    ],
    importpath = "gogames/tiles/triangles",
    visibility = ["//visibility:public"],
    deps = [
    ],
)

go_test(
    name = "triangles_test",
    srcs = [
      "triangles_test.go",
    ],
    embed = [
      ":triangles",
    ],
    deps = [
    ],
)
