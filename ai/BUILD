package(default_visibility = ["//ai:__subpackages__"])

cc_library(
  name = "unit_ai",
  srcs = ["unit_ai.cc"],
  hdrs = ["unit_ai.h"],
  deps = [
    "//actions/proto:strategy_proto",
    "//actions/proto:plan_proto",
    "//units:units",
  ],
)

cc_library(
  visibility = ["//visibility:public"],
  name = "planner",
  srcs = ["planner.cc"],
  hdrs = ["planner.h"],
  deps = [
    ":unit_ai",
    "//ai/impl:unit_ai_impl",
    "//units:units",
  ],
)
