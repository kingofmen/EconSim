package(default_visibility = ["//visibility:public"])

cc_library(
    name = "geography",
    srcs = ["geography.cc"],
    hdrs = ["geography.h"],
    deps = [
        "//games/geography/proto:geography_proto",
        "//games/industry:industry",
        "//games/market/proto:goods_proto",
        "//util/logging:logging",
        "//util/proto:object_id",
        "//util/status:status",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "geography_test",
    srcs = ["geography_test.cc"],
    deps = [
        ":geography",
        "//util/proto:object_id",
        "//util/status:status",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)

cc_library(
    name = "mobile",
    hdrs = ["mobile.h"],
    deps = [
        "//games/geography/proto:geography_proto",
        "//util/headers:int_types",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "connection",
    srcs = ["connection.cc"],
    hdrs = ["connection.h"],
    deps = [
        ":geography",
        ":mobile",
        "//games/geography/proto:geography_proto",
        "//util/headers:int_types",
        "//util/proto:object_id",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "connection_test",
    srcs = ["connection_test.cc"],
    deps = [
        ":connection",
        ":mobile",
        "//games/geography/proto:geography_proto",
        "//util/headers:int_types",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)
