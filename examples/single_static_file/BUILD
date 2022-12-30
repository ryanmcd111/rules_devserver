load("//devserver:defs.bzl", "devserver")

devserver(
    name = "serve",
    port = 8081,
    workspace_name = "rules_devserver",
    static_files = {
        "/": ["//examples/single_static_file/static:index", "index.html"]
    },
    debug = True,
    data = ["//examples/single_static_file/static:index"]
)