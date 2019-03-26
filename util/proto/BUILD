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
