# rules_devserver: Web Development Server Bazel Rule

This repository contains a Bazel rule for running a development server. It is
intended to be used for local development, and is not intended for production
use.

## Features

*   Live-reload via `ibazel` and a custom live-reload script.
*   Hot swappable JavaScript modules.
*   Custom routes for any dependency in the BUILD rule's `data` attribute.

## Pre-requisites

* Bazel 6.0.0 or higher
* ibazel for live-reload

## Setup

### Workspace

```
workspace(name = "test_rules_devserver")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
http_archive(
    name = "rules_devserver",
    strip_prefix = "rules_devserver-0.1.0",
    urls = ["https://github.com/ryanmcdermott/rules_devserver/archive/refs/tags/0.1.0.tar.gz"],
)
```

### Example BUILD file

```
load("@rules_devserver//devserver:defs.bzl", "devserver")

filegroup(
    name = "index",
    srcs = ["index.html"],
    visibility = ["//visibility:public"]
)

filegroup(
    name = "bundle"
    srcs = ["bundle.js"],
    visibility = ["//visibility:public"]
)

devserver(
    name = "serve",
    port = 8081,
    workspace_name = "test_rules_devserver",
    static_files = {
        "/": [":index", "index.html"],
        "/scripts/bundle.js": [":bundle", "bundle.js"]
    },
    data = [":index", ":bundle"]
)
```

The `static_files` attribute is a dictionary of routes to files. The key is the route and the value is a tuple
of the [filegroup target, file name]. All targets in the `static_files` attribute must also be in the `data` attribute.

## Usage of Example BUILD file

```
ibazel run //:serve
```

Whenever a file in the `data` attribute is changed, the server will be live-reloaded.

## Hot Swap JavaScript Modules.

To hot swap a JavaScript module, add the a data attribute on the `<script>` tag in the HTML file. For example:

```
<script type="module" data-hot-swap="true" src="/scripts/bundle.js"></script>
```

When the server is live-reloaded, the browser will hot swap the JavaScript module, and the hot page itself will not reload.