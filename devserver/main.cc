#include <iostream>

// #include "external/rules_devserver/devserver/argparse/argparse.h"
// #include "external/rules_devserver/devserver/httplib/httplib.h"
// #include "external/rules_devserver/devserver/json/json.h"

#include "devserver/argparse/argparse.h"
#include "devserver/httplib/httplib.h"
#include "devserver/json/json.h"
#include "tools/cpp/runfiles/runfiles.h"

using bazel::tools::cpp::runfiles::Runfiles;

#define DEBUG true
#define DEBUG_LOG(msg) \
  if (DEBUG) std::cout << msg << std::endl;

constexpr char kWorkspaceName[] = "rules_devserver";
constexpr int kDefaultPort = 8080;
constexpr char kHost[] = "localhost";

struct Arguments {
  int32_t port;
  std::string static_file;
  std::string workspace_name;
  std::string package_name;
};

std::string GetFileContents(const std::string &path) {
  std::ifstream ifs(path);
  std::string content((std::istreambuf_iterator<char>(ifs)),
                      (std::istreambuf_iterator<char>()));
  return content;
}

std::string GetDevserverLoaderScriptContents(
    const std::string &workspace_root) {
  const std::string devserver_loader_path =
      workspace_root + "devserver/devserver_loader.js";
  DEBUG_LOG("devserver_loader_path: " << devserver_loader_path);

  std::string devserver_loader_contents;
  devserver_loader_contents = GetFileContents(devserver_loader_path);

  return devserver_loader_contents;
}

std::string AddDevserverLoaderToStaticFileContents(
    const std::string &static_file_contents) {
  const std::regex re("<\\/head>");
  const std::string replacement =
      "<script src=\"/devserver_loader.js\"></script></head>";
  std::string static_file_contents_with_devserver_loader =
      std::regex_replace(static_file_contents, re, replacement);

  DEBUG_LOG("static_file_contents: " << static_file_contents);
  return static_file_contents_with_devserver_loader;
}

std::string GetStaticFileContents(const std::string &workspace_root,
                                  const std::string &package_name,
                                  const std::string &static_file) {
  const std::string static_file_path =
      workspace_root + package_name + "/" + static_file;
  DEBUG_LOG("static_file_path: " << static_file_path);

  std::string static_file_contents;
  static_file_contents = GetFileContents(static_file_path);

  return static_file_contents;
}

Arguments ParseArguments(int argc, char **argv) {
  std::string workspace_name = kWorkspaceName;
  args::ArgumentParser parser("This is a test program.",
                              "This goes after the options.");
  args::ValueFlag<int32_t> port(parser, "port", "Server port", {"port"},
                                kDefaultPort);
  args::ValueFlag<std::string> static_file(
      parser, "static_file", "Index file to serve from top-level / route",
      {"static_file"});
  args::ValueFlag<std::string> workspace_name_arg(
      parser, "workspace_name", "Calling Bazel workspace name",
      {"workspace_name"});
  args::ValueFlag<std::string> package_name(parser, "package_name",
                                            "Package name", {"package_name"});

  parser.ParseCLI(argc, argv);

  if (port) {
    DEBUG_LOG("port: " << args::get(port));
  }
  if (static_file) {
    DEBUG_LOG("static_file: " << args::get(static_file));
  }
  if (workspace_name_arg) {
    if (args::get(workspace_name_arg) != "@" &&
        !args::get(workspace_name_arg).empty()) {
      workspace_name = args::get(workspace_name_arg);
    }

    DEBUG_LOG("workspace_name: " << workspace_name);
  }
  if (package_name) {
    DEBUG_LOG("package_name: " << args::get(package_name));
  }

  return Arguments{args::get(port), args::get(static_file), workspace_name,
                   args::get(package_name)};
}

int main(int argc, char **argv) {
  httplib::Server svr;

  std::string runfiles_error;
  std::unique_ptr<Runfiles> runfiles(
      Runfiles::Create(argv[0], BAZEL_CURRENT_REPOSITORY, &runfiles_error));

  const Arguments args = ParseArguments(argc, argv);
  const int32_t port = args.port;
  const std::string package_name = args.package_name;
  const std::string static_file = args.static_file;
  const std::string workspace_name = args.workspace_name;

  const std::string workspace_root = runfiles->Rlocation(workspace_name + "/");
  DEBUG_LOG("workspace_root: " << workspace_root << "\n\n");

  std::string static_file_contents =
      GetStaticFileContents(workspace_root, package_name, static_file);
  static_file_contents =
      AddDevserverLoaderToStaticFileContents(static_file_contents);

  svr.Get("/", [&static_file_contents](const httplib::Request &req,
                                       httplib::Response &res) {
    res.set_content(static_file_contents, "text/html");
  });

  svr.Get("/devserver_loader.js", [&workspace_root](const httplib::Request &req,
                                                    httplib::Response &res) {
    res.set_content(GetDevserverLoaderScriptContents(workspace_root),
                    "text/javascript");
  });

  svr.listen(kHost, port);
}
