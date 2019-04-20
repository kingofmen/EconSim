package(default_visibility = ["//visibility:public"])

cc_library(
    name = "file",
    srcs = ["file.cc"],
    hdrs = ["file.h"],
    deps = [
        "//util/status",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "file_test",
    srcs = ["file_test.cc"],
    data = [
        ":test_data/industry.pb.txt",
    ],
    deps = [
        ":file",
        "//industry/proto:industry_proto",
        "//util/status",
        "@com_google_absl//absl/strings",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)

cc_proto_library(
    name = "object_id_proto",
    deps = [":object_id_proto_lib"],
)

proto_library(
    name = "object_id_proto_lib",
    srcs = ["object_id.proto"],
    deps = ["//market/proto:goods_proto_lib"],
)

cc_library(
    name = "object_id",
    hdrs = ["object_id.h"],
    deps = [
        "//util/headers:int_types",
        "//util/proto:object_id_proto",
    ],
)
