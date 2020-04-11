package(default_visibility = ["//games/colony:__subpackages__"])

cc_library(
    name = "user_interface",
    srcs = ["user_interface.cc"],
    hdrs = ["user_interface.h"],
    deps = [],
)

cc_library(
    name = "factory",
    srcs = ["factory.cc"],
    hdrs = ["factory.h"],
    deps = [
        ":user_interface",
        "//games/colony/controller:controller",
        "//games/colony/interface/impl:text_interface",
    ],
)
