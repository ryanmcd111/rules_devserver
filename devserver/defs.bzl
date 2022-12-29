def devserver(name, port, static_file, data = []):
    normalized_static_file = Label(native.repository_name() + "//" + native.package_name()).relative(static_file)
    native.cc_binary(
        name = name,
        srcs = [
            Label("@rules_devserver//devserver:main"), 
            Label("@rules_devserver//devserver:httplib")
        ],
        args = [
            "--port=%d" % port, 
            "--static_file=%s" % normalized_static_file
        ],
        data = data,
        deps = ["@bazel_tools//tools/cpp/runfiles"],
        linkopts = ["-lpthread"],
    )
