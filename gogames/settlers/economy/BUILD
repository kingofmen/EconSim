load("@rules_proto//proto:defs.bzl", "proto_library")
load("@io_bazel_rules_go//go:def.bzl", "go_library", "go_test")
load("@io_bazel_rules_go//proto:def.bzl", "go_proto_library")


go_library(
    name = "production",
    srcs = [
      "production.go",
    ],
    importpath = "gogames/settlers/economy/production",
    visibility = ["//visibility:public"],
    deps = [
      ":production_go_proto",
    ],
)

go_test(
    name = "production_test",
    srcs = [
      "production_test.go",
    ],
    embed = [
      ":production",
    ],
    deps = [
    ],
)

proto_library(
    name = "production_proto",
    srcs = [
    	 "production.proto",
    ],
)

go_proto_library(
    name = "production_go_proto",
    importpath = "gogames/settlers/economy/production_proto",
    protos = [":production_proto"],
)

go_library(
    name = "chain",
    srcs = [
      "chain.go",
    ],
    importpath = "gogames/settlers/economy/chain",
    visibility = ["//visibility:public"],
    deps = [
      ":chain_go_proto",
      "@org_golang_google_protobuf//proto:proto",
    ],
)

go_test(
    name = "chain_test",
    srcs = [
      "chain_test.go",
    ],
    embed = [
      ":chain",
    ],
    deps = [
    ],
)

proto_library(
    name = "chain_proto",
    srcs = [
    	 "chain.proto",
    ],
)

go_proto_library(
    name = "chain_go_proto",
    importpath = "gogames/settlers/economy/chain_proto",
    protos = [":chain_proto"],
)
