package(default_visibility = ["//visibility:public"])

cc_proto_library(
    name = "population_proto",
    deps = [":population_proto_lib"],
)

proto_library(
    name = "population_proto_lib",
    srcs = ["population.proto"],
    deps = [
        "//market/proto:goods_proto_lib",
    ],
)

cc_proto_library(
    name = "consumption_proto",
    deps = [":consumption_proto_lib"],
)

proto_library(
    name = "consumption_proto_lib",
    srcs = ["consumption.proto"],
    deps = [
        "//market/proto:goods_proto_lib",
    ],
)
