def devserver(name, port, static_file, workspace_name, debug = False, data = []):
    # normalized_static_file = Label(native.repository_name() + "//" + native.package_name()).relative(static_file)
    is_debug = 1 if debug else 0
    native.cc_binary(
        name = name,
        srcs = [
            Label("@rules_devserver//devserver:main"), 
        ],
        args = [
            "--port=%d" % port, 
            "--static_file=%s" % static_file,
            "--workspace_name=%s" % workspace_name,
            "--package_name=%s" % native.package_name(),
            "--debug=%d" % is_debug,
        ],
        data = data + [Label("@rules_devserver//devserver:devserver_loader")],
        deps = ["@bazel_tools//tools/cpp/runfiles"],
        linkopts = ["-lpthread"],
        copts = ["-fpermissive"],
    )
