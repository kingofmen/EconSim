package(default_visibility = ["//visibility:public"])

cc_library(
  name = "market",
  srcs = ["market.cc"],
  hdrs = ["market.h"],
  deps = [
    ":goods_utils",
    "//market/proto:goods_proto",
    "//market/proto:market_proto",
  ],
)

cc_library(
  name = "goods_utils",
  srcs = ["goods_utils.cc"],
  hdrs = ["goods_utils.h"],
  deps = [
    "//market/proto:goods_proto",
  ],
)

cc_test(
  name = "goods_utils_test",
  srcs = ["goods_utils_test.cc"],
  deps = [
    ":goods_utils",
    "@gtest//:gtest",
    "@gtest//:gtest_main",
  ],
)
