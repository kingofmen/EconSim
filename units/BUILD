package(default_visibility = ["//visibility:public"])

cc_library(
  name = "mobile",
  srcs = ["mobile.cc"],
  hdrs = ["mobile.h"],
  deps = [
    "//geography/proto:geography_proto",
    "//units/proto:units_proto",
    "//util/headers:int_types",
    "@com_google_protobuf//:protobuf",
  ],
)

cc_library(
  name = "unit_id",
  hdrs = ["unit_id.h"],
  deps = [
    "//units/proto:units_proto",
    "//util/headers:int_types",
  ],
)
