load("@rules_proto//proto:defs.bzl", "proto_library")
load("@io_bazel_rules_go//go:def.bzl", "go_library", "go_test")
load("@io_bazel_rules_go//proto:def.bzl", "go_proto_library")

go_library(
    name = "population",
    srcs = [
      "population.go",
    ],
    importpath = "gogames/population/engine/population",
    visibility = ["//visibility:public"],
    deps = [
      ":population_go_proto",
      "//gogames/util:dna",
    ],
)

go_test(
    name = "population_test",
    srcs = [
      "population_test.go",
    ],
    embed = [
      ":population",
    ],
    deps = [
      ":population_go_proto",
    ],
)

proto_library(
    name = "population_proto",
    srcs = [
    	 "population.proto",
    ],
)

go_proto_library(
    name = "population_go_proto",
    importpath = "gogames/settlers/population/population_proto",
    protos = [":population_proto"],
)
