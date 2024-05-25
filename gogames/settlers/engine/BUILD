load("@io_bazel_rules_go//go:def.bzl", "go_library", "go_test")

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

