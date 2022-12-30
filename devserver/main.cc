#include <iostream>

#include "devserver/argparse/argparse.h"
#include "devserver/httplib/httplib.h"
#include "tools/cpp/runfiles/runfiles.h"

using bazel::tools::cpp::runfiles::Runfiles;

constexpr char kWorkspaceName[] = "rules_devserver";
constexpr int kDefaultPort = 8080;
constexpr char kHost[] = "localhost";

#define DEBUG true
#define DEBUG_LOG(msg) \
  if (DEBUG) std::cout << msg << std::endl;

std::string GetFileContents(const std::string &path) {
  std::ifstream ifs(path);
  std::string content((std::istreambuf_iterator<char>(ifs)),
                      (std::istreambuf_iterator<char>()));
  return content;
}

int main(int argc, char **argv) {
  httplib::Server svr;

  std::string workspace_name = kWorkspaceName;
  std::string runfiles_error;
  std::unique_ptr<Runfiles> runfiles(
      Runfiles::Create(argv[0], BAZEL_CURRENT_REPOSITORY, &runfiles_error));

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

  std::string path = runfiles->Rlocation(workspace_name + "/");

  std::string static_file_contents;
  const std::string static_file_path =
      path + args::get(package_name) + "/" + args::get(static_file);

  DEBUG_LOG("static_file_path: " << static_file_path)
  static_file_contents = GetFileContents(static_file_path);
  DEBUG_LOG("static_file_contents: " << static_file_contents)

  svr.Get("/", [&static_file_contents](const httplib::Request &req,
                                       httplib::Response &res) {
    res.set_content(static_file_contents, "text/html");
  });

  svr.listen(kHost, args::get(port));
}
