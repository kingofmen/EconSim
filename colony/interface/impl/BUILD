package(default_visibility = ["//colony:__subpackages__"])

cc_library(
    name = "text_interface",
    srcs = ["text_interface.cc"],
    hdrs = ["text_interface.h"],
    deps = [
        "//colony/controller:controller",
        "//colony/interface:user_interface",
        "//game:game_world",
        "//game/proto:game_world_proto",
        "//game/setup/proto:setup_proto",
        "//game/validation:validation",
        "//util/proto:file",
        "//util/status",
        "@com_google_absl//absl/strings:strings",
        "@com_google_protobuf//:protobuf",
    ],
)
