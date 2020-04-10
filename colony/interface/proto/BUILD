package(default_visibility = ["//visibility:public"])

cc_proto_library(
    name = "actions_proto",
    deps = [":actions_proto_lib"],
)

proto_library(
    name = "actions_proto_lib",
    srcs = ["actions.proto"],
    deps = [
        "//util/proto:object_id_proto_lib",
    ],
)
