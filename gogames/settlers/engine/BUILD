load("@io_bazel_rules_go//go:def.bzl", "go_library", "go_test")
load("@io_bazel_rules_go//proto:def.bzl", "go_proto_library")

go_library(
    name = "settlers",
    srcs = [
      "settlers.go",
      "board.go",
      "piece.go",
      "rules.go",
    ],
    importpath = "gogames/settlers/engine/settlers",
    visibility = ["//visibility:public"],
    deps = [
      "//gogames/settlers/economy:chain",
      "//gogames/settlers/economy:chain_go_proto",
      "//gogames/settlers/economy:consumption_go_proto",
      "//gogames/settlers/population:population_go_proto",
      "//gogames/tiles/triangles:triangles",
      "//gogames/util:coords",
      "//gogames/util:counts",
      "//gogames/util:dna",
    ],
)

go_test(
    name = "settlers_test",
    srcs = [
      "settlers_test.go",
      "board_test.go",
    ],
    embed = [
      ":settlers",
    ],
    deps = [
      "//gogames/util:coords",
    ],
)

proto_library(
    name = "piece_proto",
    srcs = [
    	 "piece.proto",
    ],
    deps = [
      "//gogames/tiles/triangles:triangles_proto",
      ":rule_proto",
    ],
)

go_proto_library(
    name = "piece_go_proto",
    importpath = "gogames/settlers/engine/piece_go_proto",
    visibility = ["//visibility:public"],
    protos = [":piece_proto"],
    deps = [
      "//gogames/tiles/triangles:triangles_go_proto",
      ":rule_go_proto",
    ],
)

proto_library(
    name = "rule_proto",
    srcs = [
    	 "rule.proto",
    ],
    deps = [
    ],
)

go_proto_library(
    name = "rule_go_proto",
    importpath = "gogames/settlers/engine/rule_go_proto",
    visibility = ["//visibility:public"],
    protos = [":rule_proto"],
    deps = [
    ],
)
