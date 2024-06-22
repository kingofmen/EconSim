load("@io_bazel_rules_go//go:def.bzl", "go_binary", "go_library")

go_binary(
    name = "trading",
    srcs = ["main.go"],
    deps = [
        ":fund",
        ":polygon",
        ":tools",
    ],
)

go_binary(
    name = "misprice",
    srcs = ["misprice.go"],
    deps = [
	":marketdata",
	":mispricelib",
        ":polygon",
        ":tools",
    ],
)

go_library(
    name = "fund",
    srcs = ["fund.go"],
    importpath = "trading/fund",
    deps = [
        ":tools",
    ],
)

go_library(
    name = "mispricelib",
    srcs = ["misprice_lib.go"],
    importpath = "trading/misprice",
    deps = [
	":marketdata",
        ":polygon",
    ],
)

go_library(
    name = "polygon",
    srcs = ["polygon.go"],
    importpath = "trading/polygon",
    deps = [
        ":tools",
    ],
)

go_library(
    name = "marketdata",
    srcs = ["marketdata.go"],
    importpath = "trading/marketdata",
    deps = [
    ],
)

go_library(
    name = "tools",
    srcs = ["tools.go"],
    importpath = "trading/tools",
    deps = [
    ],
)

go_binary(
    name = "volatility",
    srcs = ["volatility.go"],
    deps = [
    ],
)
