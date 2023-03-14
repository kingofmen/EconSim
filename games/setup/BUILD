package(default_visibility = ["//visibility:public"])

cc_library(
    name = "setup",
    srcs = ["setup.cc"],
    hdrs = ["setup.h"],
    deps = [
        "//games/actions:strategy",
        "//games/factions/proto:factions_proto",
        "//games/factions:factions",
        "//games/setup/proto:setup_proto",
        "//games/setup/validation:validation",
        "//games/geography:connection",
        "//games/geography:geography",
        "//games/market:goods_utils",
        "//games/population:population",
        "//games/population/proto:population_proto",
        "//games/units:units",
        "//util/arithmetic:microunits",
        "//util/keywords:keywords",
        "//util/proto:file",
        "//util/proto:object_id",
        "//util/status:status",
        "@com_google_absl//absl/strings:strings",
        "@com_google_protobuf//:protobuf",
    ],
)

filegroup(
    name = "testdata",
    srcs = glob(["test_data/*.pb.txt"]),
)

cc_test(
    name = "setup_test",
    srcs = ["setup_test.cc"],
    deps = [
        ":setup",
        "//games/factions/proto:factions_proto",
        "//games/geography/proto:geography_proto",
        "//util/proto:object_id",
        "//util/status:status",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
        "@com_google_protobuf//:protobuf",
    ],
    data = [":testdata"],
)
