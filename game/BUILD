cc_library(
    visibility = ["//visibility:public"],
    name = "game_world",
    srcs = ["game_world.cc"],
    hdrs = ["game_world.h"],
    deps = [
        "//actions/proto:plan_proto",
        "//actions/proto:strategy_proto",
        "//ai:executer",
        "//ai:planner",
        "//factions/proto:factions_proto",
        "//factions:factions",
        "//game/proto:game_world_proto",
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
    ],
)

cc_binary(
    name = "sinews",
    srcs = ["sinews_main.cc"],
    deps = [
        ":game_world",
        "//game/validation:validation",
        "//util/proto:file",
        "//util/status:status",
    ],
)

cc_test(
    name = "simple_econ_test",
    srcs = ["simple_econ_test.cc"],
    deps = [
        ":game_world",
        "//game/validation:validation",
        "//market:goods_utils",
        "//util/proto:file",
        "//util/status:status",
        "@com_google_absl//absl/strings:strings",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
    ],
    data = [
        ":test_data/simple.pb.txt",
        ":test_data/simple_economy.pb.txt",
        ":test_data/fixcap.pb.txt",
        ":test_data/fixcap_economy.pb.txt",
        ":test_data/trade.pb.txt",
        ":test_data/trade_economy.pb.txt",
    ],
)
