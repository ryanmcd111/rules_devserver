#include <iostream>

#include "devserver/httplib/httplib.h"
#include "tools/cpp/runfiles/runfiles.h"

using bazel::tools::cpp::runfiles::Runfiles;

constexpr char kWorkspaceName[] = "rules_devserver";
constexpr char kDefaultPort[] = "8080";
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

  std::string port = kDefaultPort;
  std::string workspace_name = kWorkspaceName;
  std::string package_name;
  std::string static_file;
  std::string static_file_contents;

  std::string runfiles_error;
  std::unique_ptr<Runfiles> runfiles(
      Runfiles::Create(argv[0], BAZEL_CURRENT_REPOSITORY, &runfiles_error));

  for (int i = 0; i < argc; i++) {
    if (std::string(argv[i]).rfind("--port", 0) != std::string::npos) {
      port = std::string(argv[i]).substr(7);
    }

    if (std::string(argv[i]).rfind("--static_file", 0) != std::string::npos) {
      static_file = std::string(argv[i]).substr(14);
    }

    if (std::string(argv[i]).rfind("--package_name", 0) != std::string::npos) {
      package_name = std::string(argv[i]).substr(15);
    }

    if (std::string(argv[i]).rfind("--workspace_name", 0) !=
        std::string::npos) {
      const std::string workspace_name_arg = std::string(argv[i]).substr(17);
      if (workspace_name_arg != "@" && !workspace_name_arg.empty()) {
        workspace_name = workspace_name_arg;
      }
    }
  }

  std::cout << "workspace_name: " << workspace_name << std::endl;
  std::cout << "package_name: " << package_name << std::endl;
  std::cout << "static_file:" << static_file << std::endl;

  std::string path = runfiles->Rlocation(workspace_name + "/");
  std::cout << "path: " << path << std::endl;

  std::cout << "\nport: " << port << std::endl;

  const std::string static_file_path = path + package_name + "/" + static_file;
  std::cout << "static_file_path: " << static_file_path << std::endl;
  static_file_contents = GetFileContents(static_file_path);
  std::cout << "static_file_contents: " << static_file_contents << std::endl;

  svr.Get("/", [&static_file_contents](const httplib::Request &req,
                                       httplib::Response &res) {
    res.set_content(static_file_contents, "text/html");
  });

  svr.listen(kHost, std::stoi(port));
}
