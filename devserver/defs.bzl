def devserver(name, port, static_file, data = []):
    # normalized_static_file = Label(native.repository_name() + "//" + native.package_name()).relative(static_file)
    native.cc_binary(
        name = name,
        srcs = [
            Label("@rules_devserver//devserver:main"), 
        ],
        args = [
            "--port=%d" % port, 
            "--static_file=%s" % static_file,
            "--workspace_name=%s" % native.repository_name(),
            "--package_name=%s" % native.package_name(),
        ],
        data = data,
        deps = ["@bazel_tools//tools/cpp/runfiles"],
        linkopts = ["-lpthread"],
    )
