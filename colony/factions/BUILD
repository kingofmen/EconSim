package(default_visibility = ["//colony:__subpackages__"])

cc_library(
    name = "factions",
    srcs = ["factions.cc"],
    hdrs = ["factions.h"],
    deps = [
        "//colony/factions/proto:factions_proto",
        "//util/arithmetic:microunits",
        "//util/headers:int_types",
    ],
)

cc_test(
    name = "factions_test",
    srcs = ["factions_test.cc"],
    deps = [
        ":factions",
        "//util/arithmetic:microunits",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)
