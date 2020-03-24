package(default_visibility = ["//visibility:public"])

cc_library(
    name = "setup",
    srcs = ["setup.cc"],
    hdrs = ["setup.h"],
    deps = [
        "//factions/proto:factions_proto",
        "//factions:factions",
        "//games/setup/proto:setup_proto",
        "//games/setup/validation:validation",
        "//geography:connection",
        "//geography:geography",
        "//population:population",
        "//population/proto:population_proto",
        "//units:units",
        "//util/proto:file",
        "//util/status",
        "@com_google_absl//absl/strings:strings",
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
        "@gtest//:gtest",
        "@gtest//:gtest_main",
        "@com_google_protobuf_cc//:protobuf",
    ],
    data = [":testdata"],
)
