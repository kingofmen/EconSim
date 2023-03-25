load("@io_bazel_rules_go//go:def.bzl", "go_binary", "go_library", "go_test")

go_library(
    name = "client_lib",
    srcs = [
      "client_lib.go",
    ],
    importpath = "gogames/raubgraf/clients/dev/clientlib",
    deps = [
      "//gogames/raubgraf/engine:handler",
      "//gogames/util:coords",
      "//gogames/raubgraf/proto:state_go_proto",
      "@org_golang_google_protobuf//encoding/prototext:prototext",
    ],
)

go_binary(
    name = "devgraf",
    srcs = [
      "dev_main.go",
    ],
    deps = [
      ":client_lib",
      "//third_party/golang.org/windows:windows",
    ],
)
