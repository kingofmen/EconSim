load("@io_bazel_rules_go//go:def.bzl", "go_binary", "go_library", "go_test")

go_library(
    name = "combat",
    srcs = [
      "combat.go",
    ],
    importpath = "gogames/combat/combat",
    deps = [
    ],
)

go_test(
    name = "combat_test",
    srcs = ["combat_test.go"],
    importpath = "gogames/combat/combat",
    embed = [
      ":combat",
    ],
)


