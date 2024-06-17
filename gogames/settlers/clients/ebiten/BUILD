load("@io_bazel_rules_go//go:def.bzl", "go_binary", "go_library")

go_binary(
    name = "ebit_client",
    embed = [":ebiten_lib"],
    importpath = "gogames/settlers/clients/ebiten",
    visibility = ["//visibility:public"],
)

go_library(
    name = "ebiten_lib",
    srcs = [
        "main.go",
	"components.go",
    ],
    importpath = "gogames/settlers/clients/ebiten",
    visibility = ["//visibility:private"],
    deps = [
        "//gogames/settlers/ai:cog",
        "//gogames/settlers/content:loader",
        "//gogames/settlers/engine:settlers",
      	"//gogames/tiles/triangles:triangles",
        "//gogames/util:dna",
        "//gogames/util:vector2d",
        "@org_golang_x_image//font/basicfont:basicfont",
        "@com_github_hajimehoshi_ebiten_v2//ebitenutil:go_default_library",
        "@com_github_hajimehoshi_ebiten_v2//inpututil:go_default_library",
        "@com_github_hajimehoshi_ebiten_v2//text",
        "@com_github_hajimehoshi_ebiten_v2//:go_default_library",
    ],
)
