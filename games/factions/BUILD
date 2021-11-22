package(default_visibility = ["//visibility:public"])

cc_library(
    name = "factions",
    srcs = ["factions.cc"],
    hdrs = ["factions.h"],
    deps = [
        "//games/factions/proto:factions_proto",
        "//util/arithmetic:bits",
        "//util/arithmetic:microunits",
        "//util/logging:logging",
        "//util/headers:int_types",
        "//util/proto:object_id",
    ],
)

cc_test(
    name = "factions_test",
    srcs = ["factions_test.cc"],
    deps = [
        ":factions",
        "//games/factions/proto:factions_proto",
        "//util/arithmetic:bits",
        "//util/arithmetic:microunits",
        "//util/proto:object_id",
        "@com_google_absl//absl/strings:strings",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)
