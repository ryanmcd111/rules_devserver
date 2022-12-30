load("//devserver:defs.bzl", "devserver")

devserver(
    name = "serve",
    port = 8081,
    workspace_name = "rules_devserver",
    static_file = "index.html",
    debug = True,
    data = ["index.html"]
)