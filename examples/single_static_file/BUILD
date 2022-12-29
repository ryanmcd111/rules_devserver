load("//devserver:defs.bzl", "devserver")

filegroup(
    name = "index",
    srcs = ["index.html"],
    visibility = ["//visibility:public"],
)

devserver(
    name = "serve",
    port = 8081,
    static_file = ":index",
    data = [":index"],
)