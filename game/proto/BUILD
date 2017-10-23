package(default_visibility = ["//visibility:public"])

cc_proto_library(
  name = "game_world_proto",
  deps = [
    ":game_world_proto_lib",
  ],
)

proto_library(
  name = "game_world_proto_lib",
  srcs = [
    "game_world.proto"
  ],
  deps = [
    "//geography/proto:geography_proto_lib",
    "//industry/proto:industry_proto_lib",
    "//market/proto:goods_proto_lib",
    "//population/proto:population_proto_lib",
  ],
)

