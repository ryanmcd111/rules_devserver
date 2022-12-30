def devserver(name, port, static_files, workspace_name, debug = False, data = []):
    is_debug = 1 if debug else 0
    
    files = []
    for path, pkg_set in static_files.items():
        print(path)
        pkg, file = pkg_set[0], pkg_set[1]
        norm_pkg = Label(native.repository_name() + "//" + native.package_name()).relative(pkg)
        pkg = str(norm_pkg)
        
        if pkg[0] == ":":
            pkg = pkg[1:]
        elif pkg[0] == "@":
            pkg = pkg[pkg.find("//") + 2:]
            pkg = pkg[:pkg.find(":")]
        else:
            fail("Expected a package name starting with @ or :")
        
        files.append(",".join([path, pkg, file]))

    static_files_arg = ""
    for file in files:
        static_files_arg += "--static_files=" + file + " "
    
    native.cc_binary(
        name = name,
        srcs = [
            Label("@rules_devserver//devserver:main"), 
        ],
        args = [
            "--port=%d" % port, 
            static_files_arg,
            "--workspace_name=%s" % workspace_name,
            "--debug=%d" % is_debug,
        ],
        data = data + [Label("@rules_devserver//devserver:devserver_loader")],
        deps = ["@bazel_tools//tools/cpp/runfiles"],
        linkopts = ["-lpthread"],
        copts = ["-fpermissive", "-w"],
    )
