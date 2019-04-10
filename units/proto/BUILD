package(default_visibility = ["//visibility:public"])

cc_proto_library(
  name = "units_proto",
  deps = [":units_proto_lib"],
)

cc_proto_library(
  name = "unit_templates_proto",
  deps = [
    ":unit_templates_proto_lib",
  ],
)

proto_library(
  name = "units_proto_lib",
  srcs = ["units.proto"],
  deps = [
    "//market/proto:goods_proto_lib",
    "//util/proto:object_id_proto_lib",
  ],
)

proto_library(
  name = "unit_templates_proto_lib",
  srcs = ["templates.proto"],
  deps = ["//market/proto:goods_proto_lib"],
)
