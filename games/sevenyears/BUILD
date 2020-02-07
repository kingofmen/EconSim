package(default_visibility = ["//games/sevenyears:__subpackages__"])

cc_binary(
    name = "sevenyears",
    srcs = ["sevenyears_main.cc"],
    deps = [
        "//game/proto:game_world_proto",
        "//game/setup/proto:setup_proto",
        "//game/validation:validation",
        "//games/sevenyears/graphics:sdl_interface",
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
