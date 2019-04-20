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
