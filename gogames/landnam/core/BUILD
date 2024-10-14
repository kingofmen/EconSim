load("@io_bazel_rules_go//go:def.bzl", "go_library", "go_test")
load("@io_bazel_rules_go//proto:def.bzl", "go_proto_library")

go_library(
    name = "game",
    srcs = [
      "game.go",
    ],
    importpath = "gogames/landnam/core/game",
    visibility = ["//visibility:public"],
    deps = [
      "//gogames/combat:tables",
      "//gogames/landnam/core:game_go_proto",
      "//gogames/landnam/population:pop_go_proto",
      "//gogames/landnam/population:pop",
      "//gogames/util:logic",
      "@org_golang_google_protobuf//proto:proto",
    ],
)

proto_library(
    name = "game_proto",
    srcs = [
    	 "game.proto",
    ],
    deps = [
      "//gogames/landnam/board:board_proto",
    ],
    visibility = ["//visibility:public"],
)

go_test(
    name = "game_test",
    srcs = [
      "game_test.go",
    ],
    embed = [
      ":game",
    ],
    deps = [
      ":game_go_proto",
    ],
)


go_proto_library(
    name = "game_go_proto",
    importpath = "gogames/landnam/core/game_go_proto",
    visibility = ["//visibility:public"],
    protos = [":game_proto"],
    deps = [
      "//gogames/landnam/board:board_go_proto",
    ],
)
