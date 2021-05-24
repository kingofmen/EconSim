package(default_visibility = ["//visibility:public"])

cc_proto_library(
    name = "strategy_proto",
    deps = [":strategy_proto_lib"],
)

proto_library(
    name = "strategy_proto_lib",
    srcs = ["strategy.proto"],
    deps = [
        "//games/geography/proto:geography_proto_lib",
        "//games/market/proto:goods_proto_lib",
        "//util/proto:object_id_proto_lib",
    ],
)

cc_proto_library(
    name = "plan_proto",
    deps = [":plan_proto_lib"],
)

proto_library(
    name = "plan_proto_lib",
    srcs = ["plan.proto"],
    deps = [
        "//util/proto:object_id_proto_lib",
    ],
)
