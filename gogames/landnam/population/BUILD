load("@io_bazel_rules_go//go:def.bzl", "go_library", "go_test")
load("@io_bazel_rules_go//proto:def.bzl", "go_proto_library")

go_library(
    name = "pop",
    srcs = [
      "pop.go",
    ],
    importpath = "gogames/landnam/population/pop",
    visibility = ["//visibility:public"],
    deps = [
      "//gogames/util:logic",
      "//gogames/util:logic_go_proto",
      "//gogames/landnam/population:pop_go_proto",
      "@org_golang_google_protobuf//proto:proto",
    ],
)

proto_library(
    name = "pop_proto",
    srcs = [
    	 "pop.proto",
    ],
    deps = [
      "//gogames/util:logic_proto",
    ],
)

go_test(
    name = "pop_test",
    srcs = [
      "pop_test.go",
    ],
    embed = [
      ":pop",
    ],
    deps = [
      ":pop_go_proto",
      "//gogames/util:logic_go_proto",
      "@org_golang_google_protobuf//testing/protocmp:protocmp",
      "@com_github_google_go_cmp//cmp:cmp",
      "@org_golang_google_protobuf//encoding/prototext:prototext",
    ],
)


go_proto_library(
    name = "pop_go_proto",
    importpath = "gogames/landnam/population/pop_go_proto",
    visibility = ["//visibility:public"],
    protos = [":pop_proto"],
    deps = [
      "//gogames/util:logic_go_proto",
    ],
)
