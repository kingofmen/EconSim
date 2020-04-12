package(default_visibility = ["//visibility:public"])

cc_library(
    name = "unit_impl",
    srcs = ["land_cargo.cc"],
    hdrs = ["land_cargo.h"],
    deps = [
        "//games/geography/proto:geography_proto",
        "//games/geography:mobile",
        "//util/headers:int_types",
        "@com_google_protobuf//:protobuf",
    ],
)
