package(default_visibility = ["//visibility:public"])

cc_library(
  name = "unit_impl",
  srcs = ["land_cargo.cc"],
  hdrs = ["land_cargo.h"],
  deps = [
    "//geography/proto:geography_proto",
    "//units:mobile",
    "//units/proto:units_proto",
    "//util/headers:int_types",
    "@com_google_protobuf//:protobuf",
  ],
)
