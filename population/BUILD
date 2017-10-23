package(default_visibility = ["//visibility:public"])

cc_library(
  name = "population",
  srcs = ["popunit.cc"],
  hdrs = ["popunit.h"],
  deps = [
    "//geography:geography",
    "//industry:industry",
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
