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
  ],
)

cc_binary(
  name = "sinews",
  srcs = ["sinews_main.cc"],
  deps = [
    ":game_world",
  ],
)
