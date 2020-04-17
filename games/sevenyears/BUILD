package(default_visibility = ["//games/sevenyears:__subpackages__"])

cc_library(
    name = "sevenyears_lib",
    srcs = ["sevenyears.cc"],
    hdrs = ["sevenyears.h"],
    deps = [
        "//games/actions/proto:plan_proto",
        "//games/actions/proto:strategy_proto",
        "//games/ai:executer",
        "//games/ai:planner",
        "//games/interface:base",
        "//games/interface/proto:config_proto",
        "//games/setup/proto:setup_proto",
        "//games/setup:setup",
        "//games/setup/validation:validation",
        "//games/sevenyears/proto:sevenyears_proto",
        "//games/geography:geography",
        "//util/logging:logging",
        "//util/status:status",
        "@com_google_absl//absl/strings:strings",
        "@com_google_protobuf//:protobuf",
        "@sdl2//:SDL2",
    ],
)

cc_binary(
    name = "sevenyears",
    srcs = ["sevenyears_main.cc"],
    deps = [
        ":sevenyears_lib",
        "//games/actions/proto:plan_proto",
        "//games/actions/proto:strategy_proto",
        "//games/ai:executer",
        "//games/ai:planner",
        "//games/setup/proto:setup_proto",
        "//games/setup:setup",
        "//games/sevenyears/graphics:sdl_interface",
        "//games/sevenyears/proto:sevenyears_proto",
        "//games/interface/proto:config_proto",
        "//util/proto:file",
        "//util/logging:logging",
        "//util/status:status",
        "@com_google_absl//absl/strings:strings",
        "@sdl2//:SDL2",
    ],
)
