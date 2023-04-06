package(default_visibility = ["//gogames/raubgraf:__subpackages__"])

load("@rules_proto//proto:defs.bzl", "proto_library")
load("@io_bazel_rules_go//proto:def.bzl", "go_proto_library")

proto_library(
    name = "state_proto",
    srcs = [
    	 "state.proto",
	 "actions.proto",
    ],
)

go_proto_library(
    name = "state_go_proto",
    importpath = "gogames/raubgraf/protos/state_proto",
    protos = [":state_proto"],
)
