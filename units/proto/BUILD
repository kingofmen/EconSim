package(default_visibility = ["//visibility:public"])

cc_proto_library(
  name = "units_proto",
  deps = [":units_proto_lib"],
)

cc_proto_library(
  name = "units_templates_proto",
  deps = [":units_templates_proto_lib"],
)

proto_library(
  name = "units_proto_lib",
  srcs = ["units.proto"],
  deps = ["//market/proto:goods_proto_lib"],
)

proto_library(
  name = "units_templates_proto_lib",
  srcs = ["templates.proto"],
  deps = ["//market/proto:goods_proto_lib"],
)
