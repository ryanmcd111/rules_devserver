load("//devserver:defs.bzl", "devserver")

filegroup(
    name = "hot_swap",
    srcs = ["hot_swap.js"],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "counter",
    srcs = ["counter.js"],
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
        "/js/hot_swap.js": [":hot_swap", "hot_swap.js"],
        "/js/counter.js": [":counter", "counter.js"]
    },
    debug = True,
    data = [":index", ":hot_swap", ":counter"]
)