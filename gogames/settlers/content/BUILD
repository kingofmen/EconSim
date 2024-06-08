load("@io_bazel_rules_go//go:def.bzl", "go_library", "go_test")

go_library(
    name = "loader",
    srcs = [
      "loader.go",
    ],
    importpath = "gogames/settlers/content/loader",
    visibility = ["//visibility:public"],
    deps = [
      "//gogames/settlers/economy:chain",
      "//gogames/settlers/economy:chain_go_proto",
      "//gogames/settlers/economy:consumption_go_proto",
      "//gogames/settlers/engine:settlers",
      "//gogames/tiles/triangles:triangles",
      "//gogames/util:coords",
      "//gogames/util:counts",
      "//gogames/util:dna",
      "@org_golang_google_protobuf//encoding/prototext:prototext",
      "@org_golang_google_protobuf//proto:proto",
    ],
)

filegroup(
    name = "testdata",
    srcs = glob([
        "testdata/**/*.pb.txt",
    ]),
)

go_test(
    name = "loader_test",
    srcs = [
      "loader_test.go",
    ],
    embed = [
      ":loader",
    ],
    deps = [
      "@org_golang_google_protobuf//testing/protocmp:protocmp",
      "@com_github_google_go_cmp//cmp:cmp",
    ],
    data = [
          ":testdata",
    ],
)

go_library(
    name = "validate",
    srcs = [
      "validate.go",
    ],
    importpath = "gogames/settlers/engine/validate",
    visibility = ["//visibility:public"],
    deps = [
      "//gogames/settlers/economy:chain",
      "//gogames/settlers/economy:chain_go_proto",
      "//gogames/settlers/economy:consumption_go_proto",
    ],
)

go_test(
    name = "validate_test",
    srcs = [
      "validate_test.go",
    ],
    embed = [
      ":validate",
    ],
    deps = [
    ],
)

