def devserver(name, port):
    native.cc_binary(
        name = name,
        srcs = [
            Label("@rules_devserver//devserver:main"), 
            Label("@rules_devserver//devserver:httplib")
        ],
        args = ["--port=%d" % port],
        linkopts = ["-lpthread"],
    )
