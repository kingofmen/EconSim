package(default_visibility = ["//visibility:public"])

cc_library(
    name = "microunits",
    srcs = ["microunits.cc"],
    hdrs = ["microunits.h"],
    deps = [
        "//market:goods_utils",
        "//market/proto:goods_proto",
        "//util/headers:int_types",
        "@com_google_protobuf//:protobuf_lite",
    ],
)

cc_test(
    name = "microunits_test",
    srcs = ["microunits_test.cc"],
    deps = [
        ":microunits",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)
