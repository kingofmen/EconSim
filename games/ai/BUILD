package(default_visibility = ["//visibility:public"])

cc_library(
    name = "unit_ai",
    srcs = ["unit_ai.cc"],
    hdrs = ["unit_ai.h"],
    deps = [
        "//games/actions/proto:plan_proto",
        "//games/actions/proto:strategy_proto",
        "//games/units:units",
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
        "//games/units:units",
        "//util/status:status",
        "@com_google_absl//absl/strings:strings",
    ],
)

cc_library(
    name = "executer",
    srcs = ["executer.cc"],
    hdrs = ["executer.h"],
    deps = [
        "//games/actions/proto:plan_proto",
        "//games/ai/impl:executor_impl",
        "//games/ai/public:cost",
        "//games/geography:connection",
        "//games/units:units",
        "//util/arithmetic:microunits",
        "//util/logging:logging",
        "//util/status:status",
        "@com_google_absl//absl/strings:strings",
    ],
)

cc_test(
    name = "executer_test",
    srcs = ["executer_test.cc"],
    deps = [
        ":executer",
        "//games/actions/proto:plan_proto",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)
