package(default_visibility = ["//visibility:public"])

cc_library(
    name = "strategy",
    srcs = ["strategy.cc"],
    hdrs = ["strategy.h"],
    deps = [
        "//games/actions/proto:strategy_proto",
        "//games/actions/proto:plan_proto",
        "//util/status:status",
        "@com_google_absl//absl/strings:strings",
    ],
)

cc_test(
    name = "strategy_test",
    srcs = ["strategy_test.cc"],
    deps = [
        ":strategy",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)
