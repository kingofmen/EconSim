package(default_visibility = ["//visibility:public"])

cc_library(
    name = "logging",
    srcs = ["logging.cc"],
    hdrs = ["logging.h"],
    deps = [
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_test(
    name = "logging_test",
    srcs = ["logging_test.cc"],
    size = "small",
    deps = [
        ":logging",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)
