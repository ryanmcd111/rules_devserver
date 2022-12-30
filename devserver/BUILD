filegroup(
    name = "main",
    srcs = glob([
        "main.cc", 
        "httplib/httplib.h", 
        "argparse/argparse.h", 
        "json/json.h",
        "md5/md5.h",
    ]),
    visibility = ["//visibility:public"],
)

filegroup(
    name = "devserver_loader",
    srcs = glob(["devserver_loader.js"]),
    visibility = ["//visibility:public"],
)