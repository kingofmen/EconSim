load("@io_bazel_rules_go//go:def.bzl", "go_binary", "go_library", "go_test")

go_library(
    name = "engine",
    srcs = [
      "engine.go",
    ],
    importpath = "gogames/raubgraf/engine/engine",
    deps = [
      ":board",
      ":pop",
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
      ":pop",
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
)

go_library(
    name = "pop",
    srcs = [
      "pop.go",
    ],
    importpath = "gogames/raubgraf/engine/pop",
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
