package(default_visibility = ["//visibility:public"])

cc_library(
    name = "microunits",
    srcs = ["microunits.cc"],
    hdrs = ["microunits.h"],
    deps = [
        "//util/headers:int_types",
        "@com_google_protobuf//:protobuf_lite",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/strings:strings",
    ],
)

cc_library(
    name = "bits",
    srcs = ["bits.cc"],
    hdrs = ["bits.h"],
    deps = [
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

cc_test(
    name = "bits_test",
    size = "small",
    srcs = ["bits_test.cc"],
    deps = [
        ":bits",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)
