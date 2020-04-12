package(default_visibility = ["//visibility:public"])

cc_library(
    name = "industry",
    srcs = ["industry.cc"],
    hdrs = ["industry.h"],
    deps = [
        "//games/industry/proto:industry_proto",
        "//games/market:market",
        "//games/market/proto:goods_proto",
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
        "//games/industry/decisions:production_evaluator",
        "//games/market:market",
        "//games/market/proto:goods_proto",
        "//util/arithmetic:microunits",
        "//util/headers:int_types",
        "//util/logging:logging",
    ],
)

cc_test(
    name = "industry_test",
    srcs = ["industry_test.cc"],
    deps = [
        ":industry",
        "//util/arithmetic:microunits",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)

cc_test(
    name = "worker_test",
    srcs = ["worker_test.cc"],
    deps = [
        ":worker",
        "//util/arithmetic:microunits",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)