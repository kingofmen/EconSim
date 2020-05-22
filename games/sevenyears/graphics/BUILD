package(default_visibility = ["//games/sevenyears:__subpackages__"])

cc_library(
    name = "bitmap",
    srcs = ["bitmap.cc"],
    hdrs = ["bitmap.h"],
    deps = [
        "//util/status:status",
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
        "//games/interface:base",
        "//games/interface/proto:config_proto",
        "//util/status:status",
    ],
)

cc_library(
    name = "sdl_interface",
    srcs = [
        "sdl_interface.cc",
        "sdl_sprites.cc",
    ],
    hdrs = [
        "sdl_interface.h",
        "sdl_sprites.h",
    ],
    deps = [
        ":sevenyears_interface",
        ":bitmap",
        "//games/sevenyears/graphics/proto:graphics_proto",
        "//games/geography:connection",
        "//games/geography:geography",
        "//games/interface:base",
        "//games/interface/proto:config_proto",
        "//games/units:units",
        "//util/headers:int_types",
        "//util/logging:logging",
        "//util/proto:object_id",
        "//util/status:status",
        "@com_google_absl//absl/strings:strings",
        "@sdl2//:SDL2",
    ],
)
