load("@io_bazel_rules_go//go:def.bzl", "go_binary", "go_library", "go_test")

go_library(
    name = "engine",
    srcs = [
      "engine.go",
    ],
    importpath = "gogames/raubgraf/engine/engine",
    deps = [
      ":board",
      ":graph",
      ":pop",
      ":war",
      "//gogames/util:flags",
    ],
)

go_test(
    name = "engine_test",
    srcs = ["engine_test.go"],
    importpath = "gogames/raubgraf/engine/engine",
    embed = [
      ":engine",
    ],
    deps = [
      ":board",
      ":econ",
      ":pop",
      "//gogames/util:coords",
      "//gogames/util:flags",
    ],
)

go_library(
    name = "building",
    srcs = [
      "building.go",
    ],
    importpath = "gogames/raubgraf/engine/building",
    deps = [
      ":pop",
    ],
)

go_library(
    name = "board",
    srcs = [
      "board.go",
    ],
    importpath = "gogames/raubgraf/engine/board",
    deps = [
      ":building",
      ":econ",
      ":graph",
      ":pop",
      "//gogames/raubgraf/proto:state_go_proto",
      "//gogames/util:coords",
      "//gogames/util:flags",
      "@org_golang_google_protobuf//proto:proto",
    ],
)

go_test(
    name = "board_test",
    srcs = ["board_test.go"],
    importpath = "gogames/raubgraf/engine/board",
    embed = [
      ":board",
    ],
    deps = [
      ":pop",
      "@org_golang_google_protobuf//testing/protocmp:protocmp",
      "@com_github_google_go_cmp//cmp:cmp",
    ],
)

go_library(
    name = "econ",
    srcs = [
      "econ.go",
    ],
    importpath = "gogames/raubgraf/engine/econ",
    deps = [
    ],
)

go_test(
    name = "econ_test",
    srcs = ["econ_test.go"],
    importpath = "gogames/raubgraf/engine/econ",
    embed = [
      ":econ",
    ],
    deps = [
      ":board",
    ],
)

go_library(
    name = "graph",
    srcs = [
      "graph.go",
    ],
    importpath = "gogames/raubgraf/engine/graph",
    deps = [
      ":war",
      "//gogames/util:coords",
    ],
)

go_test(
    name = "graph_test",
    srcs = ["graph_test.go"],
    importpath = "gogames/raubgraf/engine/graph",
    embed = [
      ":graph",
    ],
    deps = [
    ],
)

go_library(
    name = "handler",
    srcs = [
      "handler.go",
    ],
    visibility = ["//visibility:public"],
    importpath = "gogames/raubgraf/engine/handler",
    deps = [
      ":engine",
      ":graph",
      ":pop",
      ":war",
      "//gogames/raubgraf/proto:state_go_proto",
    ],
)

go_library(
    name = "mobile",
    srcs = [
      "mobile.go",
    ],
    importpath = "gogames/raubgraf/engine/mobile",
    deps = [
      "//gogames/util:coords",
    ],
)

go_test(
    name = "mobile_test",
    srcs = [
      "mobile_test.go",
    ],
    importpath = "gogames/raubgraf/engine/mobile",
    embed = [
      ":mobile",
    ],
)

go_library(
    name = "pop",
    srcs = [
      "pop.go",
    ],
    importpath = "gogames/raubgraf/engine/pop",
    deps = [
      ":war",
      "//gogames/util:coords",
    ],
)

go_test(
    name = "pop_test",
    srcs = [
      "pop_test.go",
    ],
    importpath = "gogames/raubgraf/engine/pop",
    embed = [
      ":pop",
    ],
)

go_library(
  name = "war",
    srcs = [
      "war.go",
    ],
    importpath = "gogames/raubgraf/engine/war",
    deps = [
      ":mobile",
    ],
)

go_test(
    name = "war_test",
    srcs = [
      "war_test.go",
    ],
    importpath = "gogames/raubgraf/engine/war",
    embed = [
      ":war",
    ],
)
