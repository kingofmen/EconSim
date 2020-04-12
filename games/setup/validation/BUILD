package(default_visibility = ["//visibility:public"])

cc_library(
    name = "validation",
    srcs = ["validation.cc"],
    hdrs = ["validation.h"],
    deps = [
        "//games/actions/proto:plan_proto",
        "//games/actions/proto:strategy_proto",
        "//games/setup/proto:setup_proto",
        "//games/market/proto:goods_proto",
        "//games/population:population",
        "//games/population/proto:population_proto",
        "//util/arithmetic:microunits",
        "//util/headers:int_types",
        "//util/proto:object_id",
        "@com_google_absl//absl/strings:strings",
    ],
)

cc_test(
    name = "validation_test",
    srcs = ["validation_test.cc"],
    deps = [
        ":validation",
        "@com_google_absl//absl/strings:strings",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
        "//games/market:goods_utils",
        "//util/proto:file",
    ],
    data = [
        ":test_data/bad_world.pb.txt",
        ":test_data/bad_scenario.pb.txt",
    ],
)
