package(default_visibility = ["//ai:__pkg__"])

cc_library(
    name = "unit_ai_impl",
    srcs = ["unit_ai_impl.cc"],
    hdrs = ["unit_ai_impl.h"],
    deps = [
        "//actions/proto:plan_proto",
        "//actions/proto:strategy_proto",
        "//ai:unit_ai",
        "//geography:connection",
        "//market:goods_utils",
        "//units",
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
        "//ai:planner",
        "//ai:unit_ai",
        "//geography",
        "//geography:connection",
        "//geography/proto:geography_proto",
        "//market:goods_utils",
        "//units",
        "//units/proto:unit_templates_proto",
        "//units/proto:units_proto",
        "//util/arithmetic:microunits",
        "//util/headers:int_types",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)
