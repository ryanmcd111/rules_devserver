load("//devserver:defs.bzl", "devserver")

# exports_files(["index.html"])

devserver(
    name = "serve",
    port = 8081,
    static_file = "index.html",
    data = ["index.html"]
)