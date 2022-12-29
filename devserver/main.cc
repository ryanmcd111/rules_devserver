#include "devserver/httplib/httplib.h"

int main(int argc, char **argv) {
  httplib::Server svr;
  std::string port = "8080";

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
