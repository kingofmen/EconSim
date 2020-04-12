cc_library(
    visibility = ["//visibility:public"],
    name = "game_world",
    srcs = ["game_world.cc"],
    hdrs = ["game_world.h"],
    deps = [
        "//games/actions/proto:plan_proto",
        "//games/actions/proto:strategy_proto",
        "//games/ai:executer",
        "//games/ai:planner",
        "//games/factions/proto:factions_proto",
        "//games/factions:factions",
        "//games/setup:setup",
        "//games/setup/proto:setup_proto",
        "//games/geography:connection",
        "//games/geography:geography",
        "//games/industry:industry",
        "//games/industry:worker",
        "//games/industry/decisions:production_evaluator",
        "//games/industry/proto:industry_proto",
        "//games/market/proto:goods_proto",
        "//games/population:population",
        "//games/population/proto:population_proto",
        "//games/units:units",
        "//util/arithmetic:microunits",
        "//util/logging:logging",
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
        "//games/market:goods_utils",
        "//util/logging:logging",
        "//util/proto:file",
        "//util/status:status",
        "@com_google_absl//absl/strings:strings",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
    ],
    data = [":testdata"],
)
