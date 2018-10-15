package(default_visibility = ["//visibility:public"])

cc_library(
  name = "industry",
  srcs = ["industry.cc"],
  hdrs = ["industry.h"],
  deps = [
    "//industry/proto:industry_proto",
    "//market/proto:goods_proto",
    "//market:market",
    "//util/arithmetic:microunits",
    "//util/headers:int_types",
  ],
)

cc_library(
  name = "worker",
  srcs = ["worker.cc"],
  hdrs = ["worker.h"],
  deps = [
    ":industry",
    "//industry/decisions:production_evaluator",
    "//market/proto:goods_proto",
    "//market:market",
    "//util/arithmetic:microunits",
    "//util/headers:int_types",
  ],
)

cc_test(
  name = "industry_test",
  srcs = ["industry_test.cc"],
  deps = [
    ":industry",
    "@gtest//:gtest",
    "@gtest//:gtest_main",
    "//util/arithmetic:microunits",
  ],
)

cc_test(
  name = "worker_test",
  srcs = ["worker_test.cc"],
  deps = [
    ":worker",
    "@gtest//:gtest",
    "@gtest//:gtest_main",
    "//util/arithmetic:microunits",
  ],
)
