package(default_visibility = ["//visibility:public"])

cc_library(
  name = "geography",
  srcs = ["geography.cc"],
  hdrs = ["geography.h"],
  deps = [
    "//industry:industry",
    "//market/proto:goods_proto",
    "//geography/proto:geography_proto",
    "//util/status:status",
  ],
)

cc_test(
  name = "geography_test",
  srcs = ["geography_test.cc"],
  deps = [
    ":geography",
    "//util/status:status",
    "@gtest//:gtest",
    "@gtest//:gtest_main",
  ],
)
