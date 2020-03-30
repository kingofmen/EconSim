package(default_visibility = ["//visibility:public"])

cc_proto_library(
    name = "factions_proto",
    deps = [":factions_proto_lib"],
)

proto_library(
    name = "factions_proto_lib",
    srcs = ["factions.proto"],
    deps = ["//util/proto:object_id_proto_lib"],
)
