package(default_visibility = ["//colony:__subpackages__"])

cc_library(
    name = "text_interface",
    srcs = ["text_interface.cc"],
    hdrs = ["text_interface.h"],
    deps = [
        "//colony/controller:controller",
        "//colony/graphics/proto:graphics_proto",
        "//colony/interface:user_interface",
        "//colony/interface/proto:actions_proto",
        "//industry/proto:industry_proto",
        "//games/setup/proto:setup_proto",
        "//games/setup:setup",
        "//games/setup/validation:validation",
        "//games/sinews:game_world",
        "//market:goods_utils",
        "//market:market",
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
