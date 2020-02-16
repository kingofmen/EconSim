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
        "//factions/proto:factions_proto_lib",
        "//geography/proto:geography_proto_lib",
        "//industry/proto:industry_proto_lib",
        "//market/proto:goods_proto_lib",
        "//population/proto:population_proto_lib",
        "//units/proto:unit_templates_proto_lib",
        "//units/proto:units_proto_lib",
    ],
)
