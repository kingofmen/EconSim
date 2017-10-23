cc_library(
    name = "gtest",
    srcs = [
        "gmock/gtest/src/gtest-all.cc",
        "gmock/src/gmock-all.cc",
    ],
    hdrs = glob([
        "**/*.h",
        "gmock/gtest/src/*.cc",
        "gmock/src/*.cc",
    ]),
    includes = [
        "gmock",
        "gmock/gtest",
        "gmock/gtest/include",
        "gmock/include",
    ],
    linkopts = ["-pthread"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "gtest_main",
    srcs = ["gmock/src/gmock_main.cc"],
    linkopts = ["-pthread"],
    visibility = ["//visibility:public"],
    deps = [":gtest"],
)
