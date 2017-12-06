package(default_visibility = ["//visibility:public"])

cc_library(
  name = "market",
  srcs = ["market.cc"],
  hdrs = ["market.h"],
  deps = [
    ":goods_utils",
    "//market/proto:goods_proto",
    "//market/proto:market_proto",
    "//util/arithmetic:microunits",
    "//util/headers:int_types",
  ],
)

cc_library(
  name = "goods_utils",
  srcs = ["goods_utils.cc"],
  hdrs = ["goods_utils.h"],
  deps = [
    "//market/proto:goods_proto",
    "//util/headers:int_types",
  ],
)

cc_test(
  name = "goods_utils_test",
  srcs = ["goods_utils_test.cc"],
  size = "small",
  deps = [
    ":goods_utils",
    "@gtest//:gtest",
    "@gtest//:gtest_main",
  ],
)

cc_test(
  name = "market_test",
  srcs = ["market_test.cc"],
  size = "small",
  deps = [
    ":market",
    "@gtest//:gtest",
    "@gtest//:gtest_main",
  ],
)
