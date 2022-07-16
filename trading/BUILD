load("@io_bazel_rules_go//go:def.bzl", "go_binary", "go_library")

go_binary(
    name = "trading",
    srcs = ["main.go"],
    deps = [
        ":fund",
        ":polygon",
    ],
)

go_library(
    name = "fund",
    srcs = ["fund.go"],
    importpath = "trading/fund",
    deps = [
    ],
)

go_library(
    name = "polygon",
    srcs = ["polygon.go"],
    importpath = "trading/polygon",
    deps = [
    ],
)
