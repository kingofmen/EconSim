package(default_visibility = ["//visibility:public"])

cc_proto_library(
    name = "geography_proto",
    deps = [":geography_proto_lib"],
)

proto_library(
    name = "geography_proto_lib",
    srcs = ["geography.proto"],
    deps = [
        "//industry/proto:industry_proto_lib",
        "//market/proto:goods_proto_lib",
        "//market/proto:market_proto_lib",
        "//util/proto:object_id_proto_lib",
    ],
)
