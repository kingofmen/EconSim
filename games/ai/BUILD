package(default_visibility = ["//visibility:public"])

cc_library(
    name = "unit_ai",
    srcs = ["unit_ai.cc"],
    hdrs = ["unit_ai.h"],
    deps = [
        "//actions/proto:plan_proto",
        "//actions/proto:strategy_proto",
        "//units",
        "//util/status:status",
    ],
)

cc_library(
    name = "planner",
    srcs = ["planner.cc"],
    hdrs = ["planner.h"],
    deps = [
        ":unit_ai",
        "//games/ai/impl:unit_ai_impl",
        "//units",
        "//util/status:status",
        "@com_google_absl//absl/strings:strings",
    ],
)

cc_library(
    name = "executer",
    srcs = ["executer.cc"],
    hdrs = ["executer.h"],
    deps = [
        "//actions/proto:plan_proto",
        "//games/ai/impl:executor_impl",
        "//units",
    ],
)

cc_test(
    name = "executer_test",
    srcs = ["executer_test.cc"],
    deps = [
        ":executer",
        "//actions/proto:plan_proto",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)