#include "devserver/httplib/httplib.h"
#include "tools/cpp/runfiles/runfiles.h"

using bazel::tools::cpp::runfiles::Runfiles;

int main(int argc, char **argv) {
  httplib::Server svr;
  std::string port = "8080";

  std::string runfiles_error;
  std::unique_ptr<Runfiles> runfiles(
      Runfiles::Create(argv[0], BAZEL_CURRENT_REPOSITORY, &runfiles_error));

  std::string path = runfiles->Rlocation("devserver/");
  std::cout << "path: " << path << std::endl;

  // print all args
  for (int i = 0; i < argc; i++) {
    std::cout << "argv[" << i << "] = " << argv[i] << std::endl;
  }

  for (int i = 0; i < argc; i++) {
    if (std::string(argv[i]).rfind("--port", 0) != std::string::npos) {
      port = std::string(argv[i]).substr(7);
    }
  }

  svr.Get("/", [](const httplib::Request &req, httplib::Response &res) {
    res.set_content("Hello World!", "text/html");
  });

  svr.listen("localhost", std::stoi(port));
}
