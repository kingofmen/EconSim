package(default_visibility = ["//visibility:public"])

cc_library(
  name = "population",
  srcs = ["popunit.cc"],
  hdrs = ["popunit.h"],
  deps = [
    "@com_google_absl//absl/algorithm:container",
    "//geography/proto:geography_proto",
    "//industry:industry",
    "//industry/decisions:production_evaluator",
    "//market/proto:goods_proto",
    "//population/proto:population_proto",
    "//util/keywords:keywords",
  ],
)

cc_test(
  name = "population_test",
  srcs = ["population_test.cc"],
  size = "small",
  deps = [
    ":population",
    "@gtest//:gtest",
    "@gtest//:gtest_main",
  ],
)
