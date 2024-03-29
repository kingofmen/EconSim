package(default_visibility = ["//games/sevenyears:__subpackages__"])

cc_library(
    name = "sevenyears_interfaces",
    hdrs = ["interfaces.h"],
    srcs = ["interfaces.cc"],
    deps = [
        "//games/setup:setup",
        "//games/sevenyears/proto:sevenyears_proto",
        "//util/logging:logging",
        "//util/proto:object_id_proto",
    ],
)

cc_library(
    name = "sevenyears_constants",
    hdrs = ["constants.h"],
    srcs = ["constants.cc"],
)

cc_library(
    name = "sevenyears_ai_state_handlers",
    srcs = [
        "ai_state_handlers.cc",
    ],
    hdrs = [
        "ai_state_handlers.h",
    ],
    deps = [
        ":sevenyears_interfaces",
        ":sevenyears_constants",
        "//games/actions/proto:plan_proto",
        "//games/ai:executer",
        "//games/ai/impl:utils",
        "//games/ai/public:cost",
        "//games/sevenyears/proto:ai_state_proto",
        "//games/sevenyears/proto:sevenyears_proto",
        "//games/geography:geography",
        "//util/logging:logging",
        "//util/proto:object_id_proto",
    ],
)

cc_library(
    name = "sevenyears_ai_lib",
    srcs = [
        "army_ai.cc",
        "merchant_ship_ai.cc",
        "action_cost_calculator.cc",
    ],
    hdrs = [
        "army_ai.h",
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
        "//games/ai/impl:utils",
        "//games/industry:industry",
        "//games/market:goods_utils",
        "//games/sevenyears/proto:ai_state_proto",
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
    srcs = glob([
        "test_data/*/*.pb.txt",
        "test_data/*/goldens/*/*.pb.txt",
    ]),
)

cc_library(
    name = "test_utils",
    srcs = ["test_utils.cc"],
    hdrs = ["test_utils.h"],
    deps = [
        ":sevenyears_interfaces",
        "//games/industry:industry",
        "//games/setup:setup",
        "//games/sevenyears/proto:sevenyears_proto",
        "//games/sevenyears/proto:testdata_proto",
        "//util/logging:logging",
        "//util/proto:file",
        "//util/proto:object_id_proto",
        "@com_google_absl//absl/strings:strings",
        "@gtest//:gtest",
    ],
    testonly = 1,
)

cc_test(
    name = "sevenyears_ai_test",
    srcs = ["ai_tests.cc"],
    deps = [
        ":sevenyears_ai_lib",
        ":sevenyears_ai_state_handlers",
        ":sevenyears_constants",
        ":sevenyears_interfaces",
        ":test_utils",
        "//games/actions/proto:strategy_proto",
        "//games/market:goods_utils",
        "//games/setup:setup",
        "//games/sevenyears/proto:sevenyears_proto",
        "//games/sevenyears/proto:testdata_proto",
        "//util/logging:logging",
        "//util/proto:object_id_proto",
        "@com_google_absl//absl/strings:strings",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
    ],
    data = [":testdata"],
)

cc_test(
    name = "sevenyears_test",
    srcs = ["sevenyears_test.cc"],
    deps = [
        ":sevenyears_ai_lib",
        ":sevenyears_lib",
        ":sevenyears_constants",
        ":sevenyears_interfaces",
        ":test_utils",
        "//games/actions/proto:strategy_proto",
        "//games/market:goods_utils",
        "//games/setup:setup",
        "//games/sevenyears/proto:sevenyears_proto",
        "//util/logging:logging",
        "//util/proto:object_id_proto",
        "@com_google_absl//absl/strings:strings",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
        # TODO: Get rid of this viral dependency through interface/Base.
        "@sdl2//:SDL2",
    ],
    data = [":testdata"],
)

cc_library(
    name = "sevenyears_battles",
    srcs = ["battles.cc"],
    hdrs = ["battles.h"],
    deps = [
        ":sevenyears_constants",
        "//games/geography:connection",
        "//games/factions:factions",
        "//games/market:goods_utils",
        "//games/units:units",
        "//util/logging:logging",
        "//util/proto:object_id_proto",
    ],
)

cc_test(
    name = "battles_test",
    srcs = ["battles_test.cc"],
    deps = [
        ":sevenyears_battles",
        ":test_utils",
        "//games/geography:connection",
        "//games/setup/proto:setup_proto",
        "//games/setup:setup",
        "//games/units:units",
        "//util/logging:logging",
        "//util/proto:object_id_proto",
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
        ":sevenyears_ai_state_handlers",
        ":sevenyears_battles",
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
        # TODO: Get rid of this viral dependency through interface/Base.
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
        "//games/ai/impl:utils",
        "//games/setup/proto:setup_proto",
        "//games/setup:setup",
        "//games/sevenyears/graphics:sdl_interface",
        "//games/sevenyears/proto:sevenyears_proto",
        "//games/interface/proto:config_proto",
        "//util/proto:file",
        "//util/proto:object_id_proto",
        "//util/logging:logging",
        "//util/status:status",
        "@com_google_absl//absl/strings:strings",
        "@sdl2//:SDL2",
    ],
)
