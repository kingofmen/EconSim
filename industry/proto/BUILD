package(default_visibility = ["//visibility:public"])

cc_proto_library(
  name = "industry_proto",
  deps = [":industry_proto_lib"],
)

cc_proto_library(
  name = "industry_decisions_proto",
  deps = [":industry_decisions_proto_lib"],
)

proto_library(
  name = "industry_proto_lib",
  srcs = ["industry.proto"],
  deps = ["//market/proto:goods_proto_lib"],
)

proto_library(
  name = "industry_decisions_proto_lib",
  srcs = ["decisions.proto"],
)
