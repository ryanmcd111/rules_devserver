#include "devserver/httplib/httplib.h"

int main(int argc, char **argv)
{
  httplib::Server svr;

  svr.Get("/", [](const httplib::Request &req, httplib::Response &res)
          { res.set_content("Hello World!", "text/plain"); });

  svr.listen("localhost", 8080);
}
