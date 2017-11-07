package(default_visibility = ["//visibility:public"])

cc_library(
  name = "population",
  srcs = ["popunit.cc"],
  hdrs = ["popunit.h"],
  deps = [
    "@com_google_absl//absl/algorithm:container",
    "//geography/proto:geography_proto",
    "//industry:industry",
    "//market/proto:goods_proto",
    "//population/proto:population_proto",
    "//population:production_evaluator",
  ],
)

cc_library(
  name = "production_evaluator",
  srcs = ["production_evaluator.cc"],
  hdrs = ["production_evaluator.h"],
  deps = [
    "//geography:geography",
    "//industry:industry",
    "//market:goods_utils",
    "//market:market",
    "//market/proto:goods_proto",
    "//population/proto:population_proto",
  ],
)

cc_test(
  name = "population_test",
  srcs = ["population_test.cc"],
  deps = [
    ":population",
    "@gtest//:gtest",
    "@gtest//:gtest_main",
  ],
)
