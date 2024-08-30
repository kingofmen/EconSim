load("@io_bazel_rules_go//go:def.bzl", "go_library", "go_test")
load("@io_bazel_rules_go//proto:def.bzl", "go_proto_library")

go_library(
    name = "triangles",
    srcs = [
      "triangles.go",
    ],
    importpath = "gogames/tiles/triangles",
    visibility = ["//visibility:public"],
    deps = [
      ":triangles_go_proto",
    ],
)

go_test(
    name = "triangles_test",
    srcs = [
      "triangles_test.go",
    ],
    embed = [
      ":triangles",
    ],
    deps = [
    ],
)

proto_library(
    name = "triangles_proto",
    srcs = [
    	 "triangles.proto",
    ],
)

go_proto_library(
    name = "triangles_go_proto",
    importpath = "gogames/settlers/triangles/triangles_proto",
    visibility = ["//visibility:public"],
    protos = [":triangles_proto"],
)
