package(default_visibility = ["//games/sevenyears:__subpackages__"])

cc_proto_library(
    name = "sevenyears_proto",
    deps = [
        ":sevenyears_proto_lib",
    ],
)

proto_library(
    name = "sevenyears_proto_lib",
    srcs = [
        "sevenyears.proto",
    ],
    deps = [
        "//util/proto:object_id_proto_lib",
        "//games/market/proto:goods_proto_lib",
        "//games/setup/proto:setup_proto_lib",
    ],
)
