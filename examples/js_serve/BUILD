load("//devserver:defs.bzl", "devserver")

filegroup(
    name = "hello_world",
    srcs = ["hello_world.js"],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "index",
    srcs = ["index.html"],
    visibility = ["//visibility:public"],
)

devserver(
    name = "serve",
    port = 8081,
    workspace_name = "rules_devserver",
    static_files = {
        "/": [":index", "index.html"],
        "/js/hello_world.js": [":hello_world", "hello_world.js"]
    },
    debug = True,
    data = [":index", ":hello_world"]
)