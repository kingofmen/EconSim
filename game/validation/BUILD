cc_library(
    name = "validation",
    srcs = ["validation.cc"],
    hdrs = ["validation.h"],
    deps = [
        "//actions/proto:plan_proto",
        "//actions/proto:strategy_proto",
        "//game/proto:game_world_proto",
        "//market/proto:goods_proto",
        "//population:population",
        "//population/proto:population_proto",
        "//util/arithmetic:microunits",
        "//util/headers:int_types",
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
        "//market:goods_utils",
        "//util/proto:file",
    ],
    data = [
        ":test_data/bad_world.pb.txt",
        ":test_data/bad_scenario.pb.txt",
    ],
)
