package(default_visibility = ["//games/ai:__pkg__"])

cc_library(
    name = "utils",
    srcs = ["ai_utils.cc"],
    hdrs = ["ai_utils.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//games/geography:connection",
        "//games/units:units",
        "//util/arithmetic:microunits",
        "//util/logging:logging",
        "//util/proto:object_id_proto",
        "@com_google_absl//absl/strings:strings",
    ],
)

cc_library(
    name = "unit_ai_impl",
    srcs = ["unit_ai_impl.cc"],
    hdrs = ["unit_ai_impl.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//games/actions/proto:plan_proto",
        "//games/actions/proto:strategy_proto",
        "//games/ai:unit_ai",
        "//games/geography:connection",
        "//games/units:units",
        "//util/arithmetic:microunits",
        "//util/logging:logging",
        "//util/proto:object_id_proto",
        "@com_google_absl//absl/strings:strings",
    ],
)

cc_library(
    name = "executor_impl",
    srcs = ["executor_impl.cc"],
    hdrs = ["executor_impl.h"],
    deps = [
        "//games/actions/proto:plan_proto",
        "//games/actions/proto:strategy_proto",
        "//games/ai/impl:utils",
        "//games/geography:connection",
        "//games/geography:geography",
        "//games/market:goods_utils",
        "//games/market:market",
        "//games/units:units",
        "//util/arithmetic:microunits",
        "//util/headers:int_types",
        "//util/logging:logging",
        "//util/proto:object_id_proto",
        "//util/status:status",
        "@com_google_absl//absl/strings:strings",
    ],
)

cc_test(
    name = "utils_test",
    srcs = ["utils_test.cc"],
    deps = [
        ":utils",
        "//games/geography:connection",
        "//games/geography:geography",
        "//games/geography/proto:geography_proto",
        "//games/units:units",
        "//games/units/proto:unit_templates_proto",
        "//games/units/proto:units_proto",
        "//util/arithmetic:microunits",
        "//util/headers:int_types",
        "//util/logging:logging",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)

cc_test(
    name = "unit_ai_impl_test",
    srcs = ["unit_ai_impl_test.cc"],
    deps = [
        ":unit_ai_impl",
        "//games/actions/proto:plan_proto",
        "//games/actions/proto:strategy_proto",
        "//games/ai:planner",
        "//games/ai:unit_ai",
        "//games/geography:connection",
        "//games/geography:geography",
        "//games/geography/proto:geography_proto",
        "//games/units:units",
        "//games/units/proto:unit_templates_proto",
        "//games/units/proto:units_proto",
        "//util/arithmetic:microunits",
        "//util/headers:int_types",
        "//util/logging:logging",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)

cc_test(
    name = "executor_impl_test",
    srcs = ["executor_impl_test.cc"],
    deps = [
        ":executor_impl",
        "//games/actions/proto:plan_proto",
        "//games/geography:connection",
        "//games/geography:geography",
        "//games/geography/proto:geography_proto",
        "//games/units:units",
        "//games/units/proto:unit_templates_proto",
        "//games/units/proto:units_proto",
        "//util/arithmetic:microunits",
        "//util/headers:int_types",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)
