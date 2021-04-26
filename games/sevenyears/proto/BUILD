package(default_visibility = ["//games/sevenyears:__subpackages__"])

cc_proto_library(
    name = "sevenyears_proto",
    deps = [
        ":sevenyears_proto_lib",
    ],
)

cc_proto_library(
    name = "ai_state_proto",
    deps = [
        ":ai_state_proto_lib",
    ],
)

cc_proto_library(
    name = "testdata_proto",
    deps = [
        ":testdata_proto_lib",
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

proto_library(
    name = "ai_state_proto_lib",
    srcs = [
        "ai_state.proto",
    ],
    deps = [
    ],
)

proto_library(
    name = "testdata_proto_lib",
    srcs = [
        "testdata.proto",
    ],
    deps = [
        ":sevenyears_proto_lib",
        "//games/units/proto:units_proto_lib",
    ],
)
