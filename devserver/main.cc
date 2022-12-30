#include <iostream>

#include "devserver/httplib/httplib.h"
#include "tools/cpp/runfiles/runfiles.h"

using bazel::tools::cpp::runfiles::Runfiles;

constexpr char kWorkspaceName[] = "rules_devserver";
constexpr char kDefaultPort[] = "8080";

int main(int argc, char **argv) {
  httplib::Server svr;
  std::string port = std::string(kDefaultPort);
  std::string static_file;

  std::string runfiles_error;
  std::unique_ptr<Runfiles> runfiles(
      Runfiles::Create(argv[0], BAZEL_CURRENT_REPOSITORY, &runfiles_error));

  std::string path = runfiles->Rlocation("rules_devserver/");
  std::cout << "path: " << path << std::endl;

  for (int i = 0; i < argc; i++) {
    std::cout << "argv[" << i << "] = " << argv[i] << std::endl;

    if (std::string(argv[i]).rfind("--port", 0) != std::string::npos) {
      port = std::string(argv[i]).substr(7);
    }

    if (std::string(argv[i]).rfind("--static_file", 0) != std::string::npos) {
      static_file = std::string(argv[i]).substr(14);
    }
  }

  std::cout << "static_file:" << static_file << std::endl;

  svr.Get("/", [](const httplib::Request &req, httplib::Response &res) {
    res.set_content("Hello World!", "text/html");
  });

  svr.listen("localhost", std::stoi(port));
}
