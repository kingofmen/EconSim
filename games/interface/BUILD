package(default_visibility = ["//visibility:public"])

cc_library(
    name = "base",
    srcs = ["base.cc"],
    hdrs = ["base.h"],
    deps = [
        "//games/interface/proto:config_proto",
        "//util/proto:object_id_proto",
        "//util/status:status",
        "@com_google_protobuf//:protobuf",
        "@sdl2//:SDL2",
    ],
)
