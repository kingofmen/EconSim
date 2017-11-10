package(default_visibility = ["//visibility:public"])

cc_library(
  name = "production_evaluator",
  srcs = ["production_evaluator.cc"],
  hdrs = ["production_evaluator.h"],
  deps = [
    "@com_google_absl//absl/strings:strings",
    "//geography:geography",
    "//industry:industry",
    "//industry/proto:industry_decisions_proto",
    "//market:goods_utils",
    "//market:market",
    "//market/proto:goods_proto",
  ],
)

cc_test(
  name = "production_evaluator_test",
  srcs = ["production_evaluator_test.cc"],
  deps = [
    ":production_evaluator",
    "@gtest//:gtest",
    "@gtest//:gtest_main",
  ],
)
