#include <iostream>
#include <map>
#include <sstream>
#include <vector>

// #include "external/rules_devserver/devserver/argparse/argparse.h"
// #include "external/rules_devserver/devserver/base64/base64.h"
// #include "external/rules_devserver/devserver/httplib/httplib.h"
// #include "external/rules_devserver/devserver/json/json.h"
// #include "external/rules_devserver/devserver/md5/md5.h"

#include "devserver/argparse/argparse.h"
#include "devserver/base64/base64.h"
#include "devserver/httplib/httplib.h"
#include "devserver/json/json.h"
#include "devserver/md5/md5.h"
#include "tools/cpp/runfiles/runfiles.h"

using bazel::tools::cpp::runfiles::Runfiles;
using ::nlohmann::json;

using Path = std::string;
using FileContents = std::string;

using PathMap = std::map<Path, FileContents>;

bool DEBUG = true;

#define DEBUG_LOG(msg) \
  if (DEBUG) std::cout << msg << std::endl;

constexpr char kWorkspaceName[] = "rules_devserver";
constexpr int kDefaultPort = 8080;
constexpr char kHost[] = "localhost";

struct Arguments {
  int32_t port;
  std::vector<std::string> static_files;
  std::string workspace_name;
};

std::string GetFileContents(const Path &path) {
  std::ifstream ifs(path);
  std::string content((std::istreambuf_iterator<char>(ifs)),
                      (std::istreambuf_iterator<char>()));
  return content;
}

std::string ComputeMimeType(const std::string &path) {
  const std::regex re("\\.([a-zA-Z0-9]+)$");
  std::smatch match;
  std::regex_search(path, match, re);
  std::string mime_type;

  if (match.size() == 2) {
    const std::string extension = match[1];
    if (extension == "html") {
      mime_type = "text/html";
    } else if (extension == "js") {
      mime_type = "text/javascript";
    } else if (extension == "css") {
      mime_type = "text/css";
    } else if (extension == "png") {
      mime_type = "image/png";
    } else if (extension == "jpg") {
      mime_type = "image/jpeg";
    } else if (extension == "gif") {
      mime_type = "image/gif";
    } else if (extension == "svg") {
      mime_type = "image/svg+xml";
    } else if (extension == "ico") {
      mime_type = "image/x-icon";
    } else if (extension == "json") {
      mime_type = "application/json";
    } else if (extension == "txt") {
      mime_type = "text/plain";
    } else if (extension == "xml") {
      mime_type = "text/xml";
    } else if (extension == "pdf") {
      mime_type = "application/pdf";
    } else if (extension == "zip") {
      mime_type = "application/zip";
    } else if (extension == "gz") {
      mime_type = "application/gzip";
    } else if (extension == "tar") {
      mime_type = "application/x-tar";
    } else if (extension == "wav") {
      mime_type = "audio/wav";
    } else if (extension == "mp3") {
      mime_type = "audio/mpeg";
    } else if (extension == "mp4") {
      mime_type = "video/mp4";
    } else if (extension == "webm") {
      mime_type = "video/webm";
    } else if (extension == "ogg") {
      mime_type = "audio/ogg";
    } else if (extension == "ogv") {
      mime_type = "video/ogg";
    } else if (extension == "flac") {
      mime_type = "audio/flac";
    } else if (extension == "woff") {
      mime_type = "font/woff";
    } else if (extension == "woff2") {
      mime_type = "font/woff2";
    } else if (extension == "ttf") {
      mime_type = "font/ttf";
    } else if (extension == "eot") {
      mime_type = "font/eot";
    } else if (extension == "otf") {
      mime_type = "font/otf";
    } else if (extension == "map") {
      mime_type = "application/json";
    } else if (extension == "woff") {
      mime_type = "font/woff";
    } else if (extension == "woff2") {
      mime_type = "font/woff2";
    } else if (extension == "ttf") {
      mime_type = "font/ttf";
    } else if (extension == "eot") {
      mime_type = "font/eot";
    } else if (extension == "otf") {
      mime_type = "font/otf";
    } else if (extension == "map") {
      mime_type = "application/json";
    } else {
      mime_type = "application/octet-stream";
    }
  }

  return mime_type;
}

json ComputeManifest(const PathMap &path_to_contents) {
  json manifest;

  for (const auto &path_and_contents : path_to_contents) {
    const Path path = path_and_contents.first;
    const FileContents contents = path_and_contents.second;

    picohash_ctx_t ctx;
    char digest[PICOHASH_MD5_DIGEST_LENGTH];

    picohash_init_md5(&ctx);
    picohash_update(&ctx, contents.c_str(), contents.size());
    picohash_final(&ctx, digest);
    const std::string digest_str(
        base64_encode(digest, PICOHASH_MD5_DIGEST_LENGTH));
    manifest[path] = digest_str;
  }

  return manifest;
}

std::string GetDevserverLoaderScriptContents(
    const std::string &runfiles_root, const std::string &workspace_root) {
  const std::string devserver_loader_path = runfiles_root + "/" +
                                            std::string(kWorkspaceName) +
                                            "/devserver/devserver_loader.js";
  DEBUG_LOG("devserver_loader_path: " << devserver_loader_path);

  std::string devserver_loader_contents;
  devserver_loader_contents = GetFileContents(devserver_loader_path);

  return devserver_loader_contents;
}

std::string AddDevserverLoaderToStaticFileContents(
    const std::string &static_file_contents) {
  const std::regex re("<\\/head>");
  const std::string replacement =
      "<script src=\"/devserver/devserver_loader.js\"></script></head>";
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

PathMap ComputePathMap(const std::string &workspace_root,
                       const std::vector<std::string> &static_files) {
  PathMap path_map;

  for (const auto &static_file : static_files) {
    std::stringstream test(static_file);
    std::string segment;
    std::vector<std::string> seglist;

    while (std::getline(test, segment, ',')) {
      seglist.push_back(segment);
    }

    const std::string path = seglist[0];
    const std::string package_name = seglist[1];
    const std::string file = seglist[2];

    std::string contents =
        GetStaticFileContents(workspace_root, package_name, file);

    if (file.find(".html") != std::string::npos) {
      contents = AddDevserverLoaderToStaticFileContents(contents);
    }

    path_map[path] = contents;
  }

  return path_map;
}

Arguments ParseArguments(int argc, char **argv) {
  std::string workspace_name = kWorkspaceName;
  args::ArgumentParser parser("rules_devserver",
                              "Bazel web development server.");
  args::ValueFlag<int32_t> port(parser, "port", "Server port", {"port"},
                                kDefaultPort);
  args::ValueFlagList<std::string> static_files(
      parser, "static_files", "Files to serve per route", {"static_files"});
  args::ValueFlag<std::string> workspace_name_arg(
      parser, "workspace_name", "Calling Bazel workspace name",
      {"workspace_name"});
  args::ValueFlag<bool> debug(parser, "debug", "Debug mode", {"debug"});

  parser.ParseCLI(argc, argv);

  if (port) {
    DEBUG_LOG("port: " << args::get(port));
  }
  if (static_files) {
    for (const auto &static_file : args::get(static_files)) {
      DEBUG_LOG("static_file: " << static_file);
    }
  }
  if (workspace_name_arg) {
    if (args::get(workspace_name_arg) != "@" &&
        !args::get(workspace_name_arg).empty()) {
      workspace_name = args::get(workspace_name_arg);
    }

    DEBUG_LOG("workspace_name: " << workspace_name);
  }

  return Arguments{args::get(port), args::get(static_files), workspace_name};
}

int main(int argc, char **argv) {
  httplib::Server svr;

  std::string runfiles_error;
  std::unique_ptr<Runfiles> runfiles(
      Runfiles::Create(argv[0], BAZEL_CURRENT_REPOSITORY, &runfiles_error));

  const Arguments args = ParseArguments(argc, argv);
  const int32_t port = args.port;
  const std::vector<std::string> static_files = args.static_files;
  const std::string workspace_name = args.workspace_name;

  std::string workspace_root = runfiles->Rlocation(workspace_name);
  const std::string runfiles_root =
      workspace_root.substr(0, workspace_root.find_last_of("/"));
  workspace_root = workspace_root + "/";
  DEBUG_LOG("runfiles_root: " << runfiles_root);
  DEBUG_LOG("workspace_root: " << workspace_root << "\n\n");

  const PathMap path_map = ComputePathMap(workspace_root, static_files);

  json manifest = ComputeManifest(path_map);
  DEBUG_LOG("manifest: " << manifest.dump() << "\n\n");

  for (auto &entry : path_map) {
    const std::string path = entry.first;
    const std::string contents = entry.second;

    svr.Get(path.c_str(), [path, contents](const httplib::Request &req,
                                           httplib::Response &res) {
      DEBUG_LOG("path: " << path << "\n");
      DEBUG_LOG("contents: " << contents << "\n\n");
      res.set_content(contents, ComputeMimeType(path));
    });
  }
  svr.Get("/devserver/devserver_loader.js",
          [&runfiles_root, &workspace_root](const httplib::Request &req,
                                            httplib::Response &res) {
            res.set_content(
                GetDevserverLoaderScriptContents(runfiles_root, workspace_root),
                "text/javascript");
          });

  svr.Get("/devserver/manifest",
          [&manifest](const httplib::Request &req, httplib::Response &res) {
            res.set_content(manifest.dump(), "application/json");
          });

  svr.listen(kHost, port);
}
