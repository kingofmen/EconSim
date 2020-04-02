package(default_visibility = ["//visibility:public"])

cc_library(
    name = "units",
    srcs = ["unit.cc"],
    hdrs = ["unit.h"],
    deps = [
        "//geography:mobile",
        "//market:goods_utils",
        "//market/proto:goods_proto",
        "//units/proto:unit_templates_proto",
        "//units/proto:units_proto",
        "//util/arithmetic:microunits",
        "//util/logging:logging",
        "//util/proto:object_id",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "unit_test",
    srcs = ["unit_test.cc"],
    deps = [
        ":units",
        "//util/proto:object_id",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)
