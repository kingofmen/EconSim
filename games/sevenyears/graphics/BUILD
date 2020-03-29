package(default_visibility = ["//games/sevenyears:__subpackages__"])

cc_library(
    name = "bitmap",
    srcs = ["bitmap.cc"],
    hdrs = ["bitmap.h"],
    deps = [
        "//util/status",
        "@com_google_absl//absl/strings:strings",
        "@sdl2//:SDL2",
    ],
)

cc_library(
    name = "sevenyears_interface",
    srcs = ["sevenyears_interface.cc"],
    hdrs = ["sevenyears_interface.h"],
    deps = [
        "//games/sevenyears/graphics/proto:graphics_proto",
        "//interface:base",
        "//interface/proto:config_proto",
        "//util/status:status",
    ],
)

cc_library(
    name = "sdl_interface",
    srcs = ["sdl_interface.cc"],
    hdrs = ["sdl_interface.h"],
    deps = [
        ":sevenyears_interface",
        ":bitmap",
        "//games/sevenyears/graphics/proto:graphics_proto",
        "//geography:connection",
        "//geography:geography",
        "//interface:base",
        "//interface/proto:config_proto",
        "//units:units",
        "//util/headers:int_types",
        "//util/logging:logging",
        "//util/status:status",
        "@com_google_absl//absl/strings:strings",
        "@sdl2//:SDL2",
    ],
)
