package(default_visibility = ["//games/sevenyears:__subpackages__"])

cc_binary(
    name = "sevenyears",
    srcs = ["sevenyears_main.cc"],
    deps = [
        "//actions/proto:plan_proto",
        "//actions/proto:strategy_proto",
        "//ai:executer",
        "//ai:planner",
        "//games/setup/proto:setup_proto",
        "//games/setup:setup",
        "//games/sevenyears/graphics:sdl_interface",
        "//geography:geography",
        "//interface:base",
        "//interface/proto:config_proto",
        "//util/proto:file",
        "//util/logging:logging",
        "//util/status",
        "@com_google_absl//absl/strings:strings",
        "@com_google_protobuf//:protobuf",
        "@sdl2//:SDL2",
    ],
)
