#include <iostream>
#include <map>

// #include "external/rules_devserver/devserver/argparse/argparse.h"
// #include "external/rules_devserver/devserver/httplib/httplib.h"
// #include "external/rules_devserver/devserver/json/json.h"
// #include "external/rules_devserver/devserver/md5/md5.h"

#include "devserver/argparse/argparse.h"
#include "devserver/httplib/httplib.h"
#include "devserver/json/json.h"
#include "devserver/md5/md5.h"
#include "tools/cpp/runfiles/runfiles.h"

using bazel::tools::cpp::runfiles::Runfiles;
using ::nlohmann::json;

using Path = std::string;
using FileContents = std::string;

bool DEBUG = false;

#define DEBUG_LOG(msg) \
  if (DEBUG) std::cout << msg << std::endl;

constexpr char kWorkspaceName[] = "rules_devserver";
constexpr int kDefaultPort = 8080;
constexpr char kHost[] = "localhost";

/*
 * Base64 encoding/decoding (RFC1341)
 * Copyright (c) 2005-2011, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

// 2016-12-12 - Gaspard Petit : Slightly modified to return a std::string
// instead of a buffer allocated with malloc.

#include <string>

static const unsigned char base64_table[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
 * base64_encode - Base64 encode
 * @src: Data to be encoded
 * @len: Length of the data to be encoded
 * @out_len: Pointer to output length variable, or %NULL if not used
 * Returns: Allocated buffer of out_len bytes of encoded data,
 * or empty string on failure
 */
std::string base64_encode(const unsigned char *src, size_t len) {
  unsigned char *out, *pos;
  const unsigned char *end, *in;

  size_t olen;

  olen = 4 * ((len + 2) / 3); /* 3-byte blocks to 4-byte */

  if (olen < len) return std::string(); /* integer overflow */

  std::string outStr;
  outStr.resize(olen);
  out = (unsigned char *)&outStr[0];

  end = src + len;
  in = src;
  pos = out;
  while (end - in >= 3) {
    *pos++ = base64_table[in[0] >> 2];
    *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
    *pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
    *pos++ = base64_table[in[2] & 0x3f];
    in += 3;
  }

  if (end - in) {
    *pos++ = base64_table[in[0] >> 2];
    if (end - in == 1) {
      *pos++ = base64_table[(in[0] & 0x03) << 4];
      *pos++ = '=';
    } else {
      *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
      *pos++ = base64_table[(in[1] & 0x0f) << 2];
    }
    *pos++ = '=';
  }

  return outStr;
}

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

json ComputeManifest(const std::map<Path, FileContents> &path_to_contents) {
  json manifest;

  for (const auto &path_and_contents : path_to_contents) {
    Path path = path_and_contents.first;
    FileContents contents = path_and_contents.second;

    picohash_ctx_t ctx;
    char digest[PICOHASH_MD5_DIGEST_LENGTH];

    picohash_init_md5(&ctx);
    picohash_update(&ctx, &contents, contents.size());
    picohash_final(&ctx, digest);
    std::string digest_str(base64_encode(digest, PICOHASH_MD5_DIGEST_LENGTH));
    manifest[path] = digest_str;
  }

  return manifest;
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
      parser, "workspace_name", "Cgalling Bazel workspace name",
      {"workspace_name"});
  args::ValueFlag<std::string> package_name(parser, "package_name",
                                            "Package name", {"package_name"});
  args::ValueFlag<bool> debug(parser, "debug", "Debug mode", {"debug"}, DEBUG);

  // DEBUG = debug;
  std::cout << "DEBUG: " << DEBUG << std::endl;
  std::cout << "debug: " << debug << std::endl;

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

  const std::map<std::string, std::string> path_to_contents = {
      {"/", static_file_contents}};
  json manifest = ComputeManifest(path_to_contents);
  std::cout << "manifest: " << manifest.dump() << std::endl;

  svr.Get("/", [&static_file_contents](const httplib::Request &req,
                                       httplib::Response &res) {
    res.set_content(static_file_contents, "text/html");
  });

  svr.Get(
      "/devserver/devserver_loader.js",
      [&workspace_root](const httplib::Request &req, httplib::Response &res) {
        res.set_content(GetDevserverLoaderScriptContents(workspace_root),
                        "text/javascript");
      });

  svr.Get("/devserver/manifest",
          [&manifest](const httplib::Request &req, httplib::Response &res) {
            res.set_content(manifest.dump(), "application/json");
          });

  svr.listen(kHost, port);
}
