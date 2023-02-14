load("@io_bazel_rules_go//go:def.bzl", "go_library", "go_test")

go_library(
    name = "flags",
    srcs = [
      "flags.go",
    ],
    importpath = "gogames/raubgraf/util/flags",
    deps = [
    ],
)

go_test(
    name = "flags_test",
    srcs = ["flags_test.go"],
    importpath = "gogames/raubgraf/util/flags",
    embed = [
      ":flags",
    ],
)
