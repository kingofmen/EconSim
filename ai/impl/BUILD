package(default_visibility = ["//ai:__pkg__"])

cc_library(
  name = "unit_ai_impl",
  srcs = ["unit_ai_impl.cc"],
  hdrs = ["unit_ai_impl.h"],
  deps = [
    "//ai:unit_ai",
    "//actions/proto:strategy_proto",
    "//actions/proto:plan_proto",
    "//geography:connection",
    "//market:goods_utils",
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
    "//ai:planner",
    "//ai:unit_ai",
    "//actions/proto:strategy_proto",
    "//actions/proto:plan_proto",
    "//geography:connection",
    "//geography:geography",
    "//geography/proto:geography_proto",
    "//market:goods_utils",
    "//units:units",
    "//units/proto:units_proto",
    "//units/proto:unit_templates_proto",
    "//util/arithmetic:microunits",
    "//util/headers:int_types",
    "@gtest//:gtest",
    "@gtest//:gtest_main",
  ],
)