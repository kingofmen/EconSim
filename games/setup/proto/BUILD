package(default_visibility = ["//visibility:public"])

cc_proto_library(
    name = "setup_proto",
    deps = [
        ":setup_proto_lib",
    ],
)

proto_library(
    name = "setup_proto_lib",
    srcs = [
        "setup.proto",
    ],
    deps = [
        "//games/factions/proto:factions_proto_lib",
        "//games/geography/proto:geography_proto_lib",
        "//games/industry/proto:industry_proto_lib",
        "//games/market/proto:goods_proto_lib",
        "//games/population/proto:population_proto_lib",
        "//games/units/proto:unit_templates_proto_lib",
        "//games/units/proto:units_proto_lib",
    ],
)
