package(default_visibility = ["//visibility:public"])

cc_proto_library(
    name = "market_proto",
    deps = [":market_proto_lib"],
)

cc_proto_library(
    name = "goods_proto",
    deps = [":goods_proto_lib"],
)

proto_library(
    name = "market_proto_lib",
    srcs = ["market.proto"],
    deps = [":goods_proto_lib"],
)

proto_library(
    name = "goods_proto_lib",
    srcs = ["goods.proto"],
)
