package(default_visibility = ["//games/ai:__pkg__"])

cc_library(
    name = "unit_ai_impl",
    srcs = ["unit_ai_impl.cc"],
    hdrs = ["unit_ai_impl.h"],
    deps = [
        "//actions/proto:plan_proto",
        "//actions/proto:strategy_proto",
        "//games/ai:unit_ai",
        "//geography:connection",
        "//market:goods_utils",
        "//units:units",
        "//util/arithmetic:microunits",
        "//util/headers:int_types",
    ],
)

cc_library(
    name = "executor_impl",
    srcs = ["executor_impl.cc"],
    hdrs = ["executor_impl.h"],
    deps = [
        "//actions/proto:plan_proto",
        "//actions/proto:strategy_proto",
        "//geography:connection",
        "//geography:geography",
        "//market:goods_utils",
        "//market:market",
        "//units:units",
        "//util/arithmetic:microunits",
        "//util/headers:int_types",
    ],
)

cc_test(
    name = "unit_ai_impl_test",
    srcs = ["unit_ai_impl_test.cc"],
    deps = [
        ":unit_ai_impl",
        "//actions/proto:plan_proto",
        "//actions/proto:strategy_proto",
        "//games/ai:planner",
        "//games/ai:unit_ai",
        "//geography:connection",
        "//geography:geography",
        "//geography/proto:geography_proto",
        "//market:goods_utils",
        "//units:units",
        "//units/proto:unit_templates_proto",
        "//units/proto:units_proto",
        "//util/arithmetic:microunits",
        "//util/headers:int_types",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)

cc_test(
    name = "executor_impl_test",
    srcs = ["executor_impl_test.cc"],
    deps = [
        ":executor_impl",
        "//actions/proto:plan_proto",
        "//geography:connection",
        "//geography:geography",
        "//geography/proto:geography_proto",
        "//market:goods_utils",
        "//units:units",
        "//units/proto:unit_templates_proto",
        "//units/proto:units_proto",
        "//util/arithmetic:microunits",
        "//util/headers:int_types",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)
