load("//mediapipe/framework/port:build_config.bzl", "mediapipe_cc_proto_library")
proto_library(
    name = "hand-gesture-recognition-calculator_proto",
    srcs = ["hand-gesture-recognition-calculator.proto"],
    visibility = ["//visibility:public"],
    deps = ["//mediapipe/framework:calculator_proto"],
)

mediapipe_cc_proto_library(
    name = "hand-gesture-recognition-calculator_cc_proto",
    srcs = ["hand-gesture-recognition-calculator.proto"],
    cc_deps = ["//mediapipe/framework:calculator_cc_proto"],
    visibility = ["//visibility:public"],
    deps = [":hand-gesture-recognition-calculator_proto"],
)

cc_library(
    name = "hand-gesture-recognition-calculator",
    srcs = ["hand-gesture-recognition-calculator.cc"],
    visibility = ["//visibility:public"],
    deps = [
        ":hand-gesture-recognition-calculator_cc_proto",
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/formats:landmark_cc_proto",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework/formats:rect_cc_proto",
        "//mediapipe/framework/port:ret_check",
        "//mediapipe/framework:packet",
 #      "//mediapipe/util/android/file/base",
 #      "//mediapipe/framework/port:file_helpers",
        "//mediapipe/util:resource_util",
        "//mediapipe/framework/formats:gesture_cc_proto",
    ] + select({
        "//mediapipe:android": [
            "//mediapipe/util/android/file/base",
        ],
        "//mediapipe:ios": [
            "//mediapipe/util/android/file/base",
        ],
        "//mediapipe:macos": [
            "//mediapipe/framework/port:file_helpers",
        ],
        "//conditions:default": [
            "//mediapipe/framework/port:file_helpers",
        ],
    }),
  alwayslink = 1,
)
