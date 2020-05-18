package(default_visibility = ["//games/sevenyears:__subpackages__"])

cc_library(
    name = "sevenyears_interfaces",
    hdrs = ["interfaces.h"],
    deps = [
        "//games/setup:setup",
        "//games/sevenyears/proto:sevenyears_proto",
        "//util/proto:object_id_proto",
    ],
)

cc_library(
    name = "sevenyears_constants",
    hdrs = ["constants.h"],
    srcs = ["constants.cc"],
)

cc_library(
    name = "sevenyears_ai_lib",
    srcs = [
        "merchant_ship_ai.cc",
        "action_cost_calculator.cc",
    ],
    hdrs = [
        "merchant_ship_ai.h",
        "action_cost_calculator.h",
    ],
    deps = [
        ":sevenyears_interfaces",
        ":sevenyears_constants",
        "//games/actions/proto:plan_proto",
        "//games/actions/proto:strategy_proto",
        "//games/ai:executer",
        "//games/ai:planner",
        "//games/ai/impl:unit_ai_impl",
        "//games/industry:industry",
        "//games/market:goods_utils",
        "//games/sevenyears/proto:sevenyears_proto",
        "//games/geography:geography",
        "//util/logging:logging",
        "//util/proto:object_id_proto",
        "//util/status:status",
        "@com_google_absl//absl/strings:strings",
        "@com_google_protobuf//:protobuf",
    ],
)

filegroup(
    name = "testdata",
    srcs = glob(["test_data/*/*.pb.txt"]),
)

cc_test(
    name = "sevenyears_ai_test",
    srcs = ["ai_tests.cc"],
    deps = [
        ":sevenyears_ai_lib",
        ":sevenyears_constants",
        ":sevenyears_interfaces",
        "//games/actions/proto:strategy_proto",
        "//games/market:goods_utils",
        "//games/setup:setup",
        "//games/sevenyears/proto:sevenyears_proto",
        "//util/logging:logging",
        "//util/proto:object_id_proto",
        "@com_google_absl//absl/strings:strings",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
    ],
    data = [":testdata"],
)

cc_library(
    name = "sevenyears_lib",
    srcs = ["sevenyears.cc"],
    hdrs = ["sevenyears.h"],
    deps = [
        ":sevenyears_ai_lib",
        ":sevenyears_constants",
        ":sevenyears_interfaces",
        "//games/actions/proto:plan_proto",
        "//games/actions/proto:strategy_proto",
        "//games/ai:executer",
        "//games/ai:planner",
        "//games/industry:industry",
        "//games/interface:base",
        "//games/interface/proto:config_proto",
        "//games/market:goods_utils",
        "//games/setup/proto:setup_proto",
        "//games/setup:setup",
        "//games/setup/validation:validation",
        "//games/sevenyears/proto:sevenyears_proto",
        "//games/geography:geography",
        "//util/proto:file",
        "//util/logging:logging",
        "//util/status:status",
        "@com_google_absl//absl/strings:strings",
        "@com_google_protobuf//:protobuf",
        "@sdl2//:SDL2",
    ],
)

cc_binary(
    name = "sevenyears",
    srcs = ["sevenyears_main.cc"],
    deps = [
        ":sevenyears_lib",
        "//games/actions/proto:plan_proto",
        "//games/actions/proto:strategy_proto",
        "//games/setup/proto:setup_proto",
        "//games/setup:setup",
        "//games/sevenyears/graphics:sdl_interface",
        "//games/sevenyears/proto:sevenyears_proto",
        "//games/interface/proto:config_proto",
        "//util/proto:file",
        "//util/logging:logging",
        "//util/status:status",
        "@com_google_absl//absl/strings:strings",
        "@sdl2//:SDL2",
    ],
)
