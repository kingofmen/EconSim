cc_library(
    visibility = ["//visibility:public"],
    name = "game_world",
    srcs = ["game_world.cc"],
    hdrs = ["game_world.h"],
    deps = [
        "//actions/proto:plan_proto",
        "//actions/proto:strategy_proto",
        "//games/ai:executer",
        "//games/ai:planner",
        "//factions/proto:factions_proto",
        "//factions:factions",
        "//games/setup:setup",
        "//games/setup/proto:setup_proto",
        "//geography:connection",
        "//geography:geography",
        "//industry:industry",
        "//industry:worker",
        "//industry/decisions:production_evaluator",
        "//industry/proto:industry_proto",
        "//market/proto:goods_proto",
        "//population:population",
        "//population/proto:population_proto",
        "//units:units",
        "//util/arithmetic:microunits",
        "//util/logging:logging",
        "//util/keywords:keywords",
        "@com_google_absl//absl/strings:strings",
    ],
)

cc_binary(
    name = "sinews",
    srcs = ["sinews_main.cc"],
    deps = [
        ":game_world",
        "//games/setup/validation:validation",
        "//util/proto:file",
        "//util/status:status",
    ],
)

filegroup(
    name = "testdata",
    srcs = glob(["test_data/*/*.pb.txt"]),
)

cc_test(
    name = "simple_econ_test",
    srcs = ["simple_econ_test.cc"],
    deps = [
        ":game_world",
        "//games/setup/validation:validation",
        "//games/setup:setup",
        "//market:goods_utils",
        "//util/logging:logging",
        "//util/proto:file",
        "//util/status:status",
        "@com_google_absl//absl/strings:strings",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
    ],
    data = [":testdata"],
)
