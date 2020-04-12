package(default_visibility = ["//games/colony:__subpackages__"])

cc_library(
    name = "text_interface",
    srcs = ["text_interface.cc"],
    hdrs = ["text_interface.h"],
    deps = [
        "//games/colony/controller:controller",
        "//games/colony/graphics/proto:graphics_proto",
        "//games/colony/interface:user_interface",
        "//games/colony/interface/proto:actions_proto",
        "//games/setup/proto:setup_proto",
        "//games/setup:setup",
        "//games/setup/validation:validation",
        "//games/sinews:game_world",
        "//games/industry/proto:industry_proto",
        "//games/market:goods_utils",
        "//games/market:market",
        "//util/arithmetic:microunits",
        "//util/logging:logging",
        "//util/proto:file",
        "//util/proto:object_id",
        "//util/proto:object_id_proto",
        "//util/status:status",
        "@com_google_absl//absl/strings:strings",
        "@com_google_protobuf//:protobuf",
    ],
)
