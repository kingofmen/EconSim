load("@io_bazel_rules_go//go:def.bzl", "go_library", "go_test")

go_library(
    name = "settlers",
    srcs = ["settlers.go"],
    importpath = "gogames/settlers/engine/settlers",
    visibility = ["//visibility:public"],
    deps = [
      "//gogames/util:coords",
    ],
)

go_test(
    name = "settlers_test",
    srcs = ["settlers_test.go"],
    embed = [
      ":settlers",
    ],
    deps = [
      "//gogames/util:coords",
    ],
)
