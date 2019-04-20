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
    size = "small",
    srcs = ["goods_utils_test.cc"],
    deps = [
        ":goods_utils",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)

cc_test(
    name = "market_test",
    size = "small",
    srcs = ["market_test.cc"],
    deps = [
        ":market",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)
