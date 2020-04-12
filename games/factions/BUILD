package(default_visibility = ["//visibility:public"])

cc_library(
    name = "factions",
    srcs = ["factions.cc"],
    hdrs = ["factions.h"],
    deps = [
        "//factions/proto:factions_proto",
        "//util/arithmetic:microunits",
        "//util/headers:int_types",
        "//util/proto:object_id",
    ],
)

cc_test(
    name = "factions_test",
    srcs = ["factions_test.cc"],
    deps = [
        ":factions",
        "//factions/proto:factions_proto",
        "//util/arithmetic:microunits",
        "//util/proto:object_id",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)
