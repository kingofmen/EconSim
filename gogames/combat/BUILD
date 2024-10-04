load("@io_bazel_rules_go//go:def.bzl", "go_binary", "go_library", "go_test")
load("@io_bazel_rules_go//proto:def.bzl", "go_proto_library")

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

proto_library(
    name = "tables_proto",
    srcs = [
    	 "tables.proto",
    ],
    deps = [
      "//gogames/util:logic_proto",
    ],
)

go_proto_library(
    name = "tables_go_proto",
    importpath = "gogames/combat/tables_go_proto",
    visibility = ["//visibility:public"],
    protos = [":tables_proto"],
    deps = [
      "//gogames/util:logic_go_proto",
    ],
)

go_library(
    name = "tables",
    srcs = [
      "tables.go",
    ],
    importpath = "gogames/combat/tables",
    deps = [
      "//gogames/util:logic",
      "//gogames/combat:tables_go_proto",
    ],
)

go_test(
    name = "tables_test",
    srcs = [
      "tables_test.go",
    ],
    embed = [
      ":tables",
    ],
    deps = [
      ":tables_go_proto",
      "//gogames/util:logic_go_proto",
      "@org_golang_google_protobuf//testing/protocmp:protocmp",
      "@com_github_google_go_cmp//cmp:cmp",
      "@org_golang_google_protobuf//encoding/prototext:prototext",
    ],
)
