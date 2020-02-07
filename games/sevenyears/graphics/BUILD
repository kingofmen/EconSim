package(default_visibility = ["//games/sevenyears:__subpackages__"])

cc_library(
    name = "sdl_interface",
    srcs = ["sdl_interface.cc"],
    hdrs = ["sdl_interface.h"],
    deps = [
        "//interface:base",
        "//interface/proto:config_proto",
        "//util/status",
        "@com_google_absl//absl/strings:strings",
        "@sdl2//:SDL2",
    ],
)
