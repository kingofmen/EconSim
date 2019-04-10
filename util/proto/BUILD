package(default_visibility = ["//visibility:public"])

cc_library(
  name = "file",
  srcs = ["file.cc"],
  hdrs = ["file.h"],
  deps = [
    "//util/status:status",
    "@com_google_protobuf//:protobuf",
  ],
)

cc_test(
  name = "file_test",
  srcs = ["file_test.cc"],
  deps = [
    ":file",
    "@com_google_absl//absl/strings:strings",
    "@gtest//:gtest",
    "@gtest//:gtest_main",
    "//industry/proto:industry_proto",
    "//util/status:status",
  ],
  data = [
    ":test_data/industry.pb.txt",
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
    "//util/proto:object_id_proto",
    "//util/headers:int_types",
  ],
)
