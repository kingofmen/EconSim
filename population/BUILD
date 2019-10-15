package(default_visibility = ["//visibility:public"])

cc_library(
    name = "population",
    srcs = ["popunit.cc"],
    hdrs = ["popunit.h"],
    deps = [
        "//geography/proto:geography_proto",
        "//industry",
        "//industry/decisions:production_evaluator",
        "//market/proto:goods_proto",
        "//population/proto:population_proto",
        "//util/headers:int_types",
        "//util/keywords",
        "@com_google_absl//absl/algorithm:container",
    ],
)

cc_library(
    name = "consumption",
    srcs = ["consumption.cc"],
    hdrs = ["consumption.h"],
    deps = [
        "//market/proto:goods_proto",
        "//market:goods_utils",
        "//market:market",
        "//population/proto:consumption_proto",
        "//util/arithmetic:microunits",
        "//util/headers:int_types",
        "//util/logging:logging",
        "//util/status:status",
        "@com_google_absl//absl/strings:strings",
    ],
)

cc_test(
    name = "population_test",
    size = "small",
    srcs = ["population_test.cc"],
    deps = [
        ":population",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)

cc_test(
    name = "consumption_test",
    size = "small",
    srcs = ["consumption_test.cc"],
    deps = [
        ":consumption",
        "//market:goods_utils",
        "//util/arithmetic:microunits",
        "//util/logging:logging",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)
