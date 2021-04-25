package(default_visibility = ["//visibility:public"])

cc_proto_library(
    name = "units_proto",
    deps = [
        ":units_proto_lib",
    ],
)

cc_proto_library(
    name = "unit_templates_proto",
    deps = [
        ":unit_templates_proto_lib",
    ],
)

proto_library(
    name = "units_proto_lib",
    srcs = ["units.proto"],
    deps = [
        "//games/actions/proto:plan_proto_lib",
        "//games/actions/proto:strategy_proto_lib",
        "//games/geography/proto:geography_proto_lib",
        "//games/market/proto:goods_proto_lib",
        "//util/proto:object_id_proto_lib",
    ],
)

proto_library(
    name = "unit_templates_proto_lib",
    srcs = ["templates.proto"],
    deps = [
        "//games/market/proto:goods_proto_lib",
        "//games/population/proto:consumption_proto_lib",
        "//util/proto:object_id_proto_lib",
    ],
)
