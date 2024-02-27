package(default_visibility = ["//visibility:public"])
load("@io_bazel_rules_go//go:def.bzl", "go_library", "go_test")

go_library(
    name = "flags",
    srcs = [
      "flags.go",
    ],
    importpath = "gogames/util/flags",
    deps = [
    ],
)

go_test(
    name = "flags_test",
    srcs = ["flags_test.go"],
    importpath = "gogames/util/flags",
    embed = [
      ":flags",
    ],
)

go_library(
    name = "coords",
    srcs = [
      "coords.go",
    ],
    importpath = "gogames/util/coords",
    deps = [
    ],
)

go_test(
    name = "coords_test",
    srcs = ["coords_test.go"],
    importpath = "gogames/util/coords",
    embed = [
      ":coords",
    ],
)

go_library(
    name = "dna",
    srcs = [
      "dna.go",
    ],
    importpath = "gogames/util/dna",
    deps = [
    ],
)

go_test(
    name = "dna_test",
    srcs = ["dna_test.go"],
    importpath = "gogames/util/dna",
    embed = [
      ":dna",
    ],
)

go_library(
    name = "vector2d",
    srcs = [
      "vector2d.go",
    ],
    importpath = "gogames/util/vector2d",
    deps = [
    ],
)

go_test(
    name = "vector2d_test",
    srcs = ["vector2d_test.go"],
    importpath = "gogames/util/vector2d",
    embed = [
      ":vector2d",
    ],
)
