package(default_visibility = ["//visibility:public"])

cc_library(
    name = "microunits",
    srcs = ["microunits.cc"],
    hdrs = ["microunits.h"],
    deps = [
        "//games/market:goods_utils",
        "//games/market/proto:goods_proto",
        "//util/headers:int_types",
        "@com_google_protobuf//:protobuf_lite",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/strings:strings",
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
