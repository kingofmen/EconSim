# Package for naval-battle movie maker.

cc_binary(
  name = "naval",
  srcs = ["naval_main.cc"],
  deps = [
     "@horde3d//:horde3d",
  ],
  linkopts = [
  	   "-DEFAULTLIB:user32.lib",
  	   "-DEFAULTLIB:gdi32.lib",
  	   "-DEFAULTLIB:shell32.lib",
  	   "-DEFAULTLIB:opengl32.lib",
  	   "-NODEFAULTLIB:libcmt.lib",
     	   "-DEFAULTLIB:msvcrt.lib",
       	   "-NODEFAULTLIB:MSVCRTD",
  ]
)

