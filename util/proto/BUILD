package(default_visibility = ["//visibility:public"])

cc_library(
    name = "file",
    srcs = ["file.cc"],
    hdrs = ["file.h"],
    deps = [
        "//util/status:status",
        "@com_google_absl//absl/strings:strings",
        "@com_google_protobuf//:protobuf",
    ],
)

filegroup(
    name = "testdata",
    srcs = glob(["test_data/*.pb.txt"]),
)

cc_test(
    name = "file_test",
    srcs = ["file_test.cc"],
    data = [
        ":testdata",
    ],
    deps = [
        ":file",
        ":object_id_proto",
        "//util/status:status",
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
    deps = ["//games/market/proto:goods_proto_lib"],
)

cc_library(
    name = "object_id",
    hdrs = ["object_id.h"],
    srcs = ["object_id.cc"],
    deps = [
        "//util/headers:int_types",
        "//util/proto:object_id_proto",
        "//util/status:status",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "object_id_test",
    srcs = ["object_id_test.cc"],
    deps = [
        ":object_id",
        "//util/status:status",
        "@com_google_absl//absl/strings",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)
