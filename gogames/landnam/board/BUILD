load("@io_bazel_rules_go//go:def.bzl", "go_library", "go_test")
load("@io_bazel_rules_go//proto:def.bzl", "go_proto_library")

go_library(
    name = "board",
    srcs = [
      "board.go",
    ],
    importpath = "gogames/landnam/board/board",
    visibility = ["//visibility:public"],
    deps = [
      "//gogames/landnam/board:board_go_proto",
      "//gogames/tiles/triangles:triangles",
    ],
)

proto_library(
    name = "board_proto",
    srcs = [
    	 "board.proto",
    ],
    deps = [
      "//gogames/landnam/population:pop_proto",
      "//gogames/tiles/triangles:triangles_proto",
    ],
    visibility = ["//visibility:public"],
)

go_test(
    name = "board_test",
    srcs = [
      "board_test.go",
    ],
    embed = [
      ":board",
    ],
    deps = [
      ":board_go_proto",
      "//gogames/landnam/population:pop_go_proto",
      "//gogames/tiles/triangles:triangles_go_proto",
      "@org_golang_google_protobuf//testing/protocmp:protocmp",
      "@com_github_google_go_cmp//cmp:cmp",
      "@org_golang_google_protobuf//encoding/prototext:prototext",
    ],
)

go_proto_library(
    name = "board_go_proto",
    importpath = "gogames/landnam/board/board_go_proto",
    visibility = ["//visibility:public"],
    protos = [":board_proto"],
    deps = [
      "//gogames/landnam/population:pop_go_proto",
      "//gogames/tiles/triangles:triangles_go_proto",
    ],
)
