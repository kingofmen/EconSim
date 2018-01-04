cc_library(
  name = "game_world",
  srcs = ["game_world.cc"],
  hdrs = ["game_world.h"],
  deps = [
    "//game/proto:game_world_proto",
    "//geography:geography",
    "//industry:industry",
    "//industry/decisions:production_evaluator",
    "//market/proto:goods_proto",
    "//population:population",
    "//population/proto:population_proto",
    "//util/arithmetic:microunits",
    "//util/keywords:keywords",
  ],
)

cc_binary(
  name = "sinews",
  srcs = ["sinews_main.cc"],
  deps = [
    ":game_world",
  ],
)

cc_test(
  name = "simple_econ_test",
  srcs = ["simple_econ_test.cc"],
  deps = [
    ":game_world",
    "@com_google_absl//absl/strings:strings",
    "@gtest//:gtest",
    "@gtest//:gtest_main",
    "//market:goods_utils",
  ],
  data = [
    ":test_data/simple.pb.txt",
    ":test_data/simple_economy.pb.txt",
  ],
)
