package(default_visibility = ["//visibility:public"])

cc_library(
    name = "production_evaluator",
    srcs = ["production_evaluator.cc"],
    hdrs = ["production_evaluator.h"],
    deps = [
        "@com_google_absl//absl/strings:strings",
        "//games/geography:geography",
        "//games/industry:industry",
        "//games/industry/proto:industry_decisions_proto",
        "//games/market:goods_utils",
        "//games/market:market",
        "//games/market/proto:goods_proto",
        "//util/arithmetic:microunits",
        "//util/headers:int_types",
        "//util/logging:logging",
    ],
)

cc_test(
    name = "production_evaluator_test",
    srcs = ["production_evaluator_test.cc"],
    deps = [
        ":production_evaluator",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
        "//util/arithmetic:microunits",
    ],
)
