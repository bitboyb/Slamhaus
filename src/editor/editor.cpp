#include "editor.hpp"
#include "../core/generator.hpp"
#include "../util/json.hpp"
#include "../util/log.hpp"
#include "../util/server.hpp"
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>

namespace Editor {

namespace {
auto GetEditableProjectRoot(const std::filesystem::path &projectRoot)
    -> std::filesystem::path {
  auto contentRoot = projectRoot / "content";
  if (std::filesystem::exists(contentRoot) &&
      std::filesystem::is_directory(contentRoot)) {
    return contentRoot;
  }

  return projectRoot;
}

auto GetMimeType(const std::string &path) -> std::string {
  std::string ext = std::filesystem::path(path).extension().string();
  if (ext == ".html")
    return "text/html";
  if (ext == ".css")
    return "text/css";
  if (ext == ".js")
    return "application/javascript";
  if (ext == ".json")
    return "application/json";
  if (ext == ".png")
    return "image/png";
  if (ext == ".jpg" || ext == ".jpeg")
    return "image/jpeg";
  if (ext == ".svg")
    return "image/svg+xml";
  if (ext == ".wasm")
    return "application/wasm";
  if (ext == ".ttf")
    return "font/ttf";
  return "application/octet-stream";
}

auto IsTextProjectFile(const std::filesystem::path &path) -> bool {
  static const std::set<std::string> textExtensions = {
      ".slam", ".smd", ".css",  ".js",   ".json", ".md",  ".txt",
      ".ini",  ".cpp", ".hpp",  ".c",    ".h",    ".yaml",".yml",
      ".html", ".htm", ".xml",  ".svg",  ".sh",   ".cmake"};
  static const std::set<std::string> textFileNames = {
      ".htaccess", "Makefile", "README", "README.md", "README.txt"};

  auto filename = path.filename().string();
  auto ext = path.extension().string();
  return textExtensions.count(ext) > 0 || textFileNames.count(filename) > 0;
}

auto IsIgnoredProjectPath(const std::filesystem::path &relativePath) -> bool {
  for (const auto &part : relativePath) {
    auto name = part.string();
    if (name == ".git" || name == "output" || name == "dist" ||
        name == "build" || name == "node_modules") {
      return true;
    }
  }
  return false;
}

auto ResolveProjectFilePath(const std::filesystem::path &projectRoot,
                            const std::string &relativePath,
                            std::filesystem::path &resolvedPath) -> bool {
  if (relativePath.empty()) {
    return false;
  }

  std::filesystem::path requested(relativePath);
  if (requested.is_absolute()) {
    return false;
  }

  auto canonicalRoot = std::filesystem::weakly_canonical(projectRoot);
  auto candidate = std::filesystem::weakly_canonical(projectRoot / requested);
  auto relative = candidate.lexically_relative(canonicalRoot);

  if (relative.empty() || relative.string().rfind("..", 0) == 0 ||
      IsIgnoredProjectPath(relative)) {
    return false;
  }

  resolvedPath = candidate;
  return true;
}

auto PruneEmptyDirectories(const std::filesystem::path &rootPath,
                           std::filesystem::path currentPath) -> void {
  while (currentPath != rootPath && std::filesystem::exists(currentPath) &&
         std::filesystem::is_directory(currentPath) &&
         std::filesystem::is_empty(currentPath)) {
    std::filesystem::remove(currentPath);
    currentPath = currentPath.parent_path();
  }
}

auto GetProjectTextFiles(const std::filesystem::path &projectRoot)
    -> std::vector<std::string> {
  std::vector<std::string> files;
  if (!std::filesystem::exists(projectRoot) ||
      !std::filesystem::is_directory(projectRoot)) {
    return files;
  }

  for (const auto &entry :
       std::filesystem::recursive_directory_iterator(projectRoot)) {
    if (!entry.is_regular_file()) {
      continue;
    }

    auto relative = std::filesystem::relative(entry.path(), projectRoot);
    if (IsIgnoredProjectPath(relative) || !IsTextProjectFile(entry.path())) {
      continue;
    }

    files.push_back(relative.generic_string());
  }

  std::sort(files.begin(), files.end());
  return files;
}
} // namespace

Session::Session() : port(0), isRunning(false), server(nullptr) {}

Session::~Session() {
  if (isRunning) {
    Stop(*this);
  }
}

auto Launch(Session &session, const std::string &projectRoot, int port,
            bool openBrowser) -> bool {
  session.projectRoot = std::filesystem::absolute(projectRoot).string();
  session.port = port;
  session.server = std::make_shared<Util::Server::ServerContext>(port);

  std::filesystem::path editorSource =
      std::filesystem::absolute("tools/system/content");
  std::filesystem::path editorDist = std::filesystem::absolute("dist/editor");

  if (std::filesystem::exists(editorDist)) {
    std::filesystem::remove_all(editorDist);
  }

  Log::Info("Building editor frontend from " + editorSource.string());
  if (!Generator::GenerateSite(editorSource, editorDist, true, true)) {
    Log::Error("Failed to build editor frontend.");
    return false;
  }

  Util::Server::RegisterRoute(
      *session.server, "GET", "/",
      [editorDist](const Util::Server::Request &req) {
        std::string subPath = req.path;
        if (subPath == "/") {
          subPath = "/index.html";
        }

        if (subPath.find("..") != std::string::npos) {
          return Util::Server::NotFound();
        }

        auto path = editorDist / subPath.substr(1);
        if (std::filesystem::exists(path)) {
          return Util::Server::File(path.string(), GetMimeType(path.string()));
        }
        return Util::Server::NotFound();
      });

  Util::Server::RegisterRoute(
      *session.server, "GET", "/preview/",
      [&session](const Util::Server::Request &req) {
        std::string subPath = req.path.substr(9);
        if (subPath.empty()) {
          subPath = "index.html";
        }

        auto projectRoot = std::filesystem::absolute(session.projectRoot);
        auto projectContent = projectRoot / "content";
        auto projectOutput = projectRoot / "output";

        if (subPath.find("assets/theme/") == 0) {
          auto sourcePath = projectContent / subPath;
          if (std::filesystem::exists(sourcePath)) {
            return Util::Server::File(sourcePath.string(),
                                      GetMimeType(sourcePath.string()));
          }
        }

        auto distPath = projectOutput / subPath;
        if (std::filesystem::exists(distPath)) {
          return Util::Server::File(distPath.string(), GetMimeType(subPath));
        }
        return Util::Server::NotFound();
      });

  Util::Server::RegisterRoute(*session.server, "GET", "/api/project/status",
                              [&session](const Util::Server::Request &req) {
                                auto projectRoot =
                                    std::filesystem::absolute(session.projectRoot);
                                auto sourceRoot =
                                    GetEditableProjectRoot(projectRoot);
                                Util::JSON::Object data;
                                data["path"] = Util::JSON::Node{
                                    projectRoot.generic_string()};
                                data["sourcePath"] = Util::JSON::Node{
                                    sourceRoot.generic_string()};
                                return Util::Server::JSON(
                                    Util::JSON::Stringify(Util::JSON::Node{data}));
                              });

  Util::Server::RegisterRoute(
      *session.server, "POST", "/api/project/set-root",
      [&session](const Util::Server::Request &req) {
        if (req.body.empty()) {
          return Util::Server::Response{400, "text/plain", "Empty Path"};
        }

        std::string newPath = req.body;
        if (std::filesystem::exists(newPath) &&
            std::filesystem::is_directory(newPath)) {
          session.projectRoot = std::filesystem::absolute(newPath).string();
          Log::Info("Project root switched to: " + session.projectRoot);
          return Util::Server::JSON("{\"success\": true}");
        }
        return Util::Server::Response{400, "text/plain", "Invalid Directory"};
      });

  Util::Server::RegisterRoute(
      *session.server, "POST", "/api/project/build",
      [&session](const Util::Server::Request &req) {
        if (Generator::GenerateProject(session.projectRoot, true, true)) {
          return Util::Server::JSON("{\"success\": true}");
        }
        return Util::Server::Response{500, "text/plain", "Build Failed"};
      });

  Util::Server::RegisterRoute(
      *session.server, "GET", "/api/project/files",
      [&session](const Util::Server::Request &req) {
        auto projectRoot = std::filesystem::absolute(session.projectRoot);
        auto sourceRoot = GetEditableProjectRoot(projectRoot);
        Util::JSON::Array items;
        for (const auto &file : GetProjectTextFiles(sourceRoot)) {
          items.push_back(Util::JSON::Node{file});
        }
        return Util::Server::JSON(
            Util::JSON::Stringify(Util::JSON::Node{items}));
      });

  Util::Server::RegisterRoute(
      *session.server, "GET", "/api/project/file/read",
      [&session](const Util::Server::Request &req) {
        auto it = req.headers.find("X-File");
        if (it == req.headers.end()) {
          return Util::Server::Response{400, "text/plain", "Missing Header"};
        }

        auto projectRoot = std::filesystem::absolute(session.projectRoot);
        auto sourceRoot = GetEditableProjectRoot(projectRoot);
        std::filesystem::path filePath;
        if (!ResolveProjectFilePath(sourceRoot, it->second, filePath) ||
            !std::filesystem::exists(filePath)) {
          return Util::Server::NotFound();
        }

        return Util::Server::File(filePath.string(), "text/plain");
      });

  Util::Server::RegisterRoute(
      *session.server, "POST", "/api/project/file/write",
      [&session](const Util::Server::Request &req) {
        auto it = req.headers.find("X-File");
        if (it == req.headers.end()) {
          return Util::Server::Response{400, "text/plain", "Missing Header"};
        }

        auto projectRoot = std::filesystem::absolute(session.projectRoot);
        auto sourceRoot = GetEditableProjectRoot(projectRoot);
        std::filesystem::path filePath;
        if (!ResolveProjectFilePath(sourceRoot, it->second, filePath) ||
            !std::filesystem::exists(filePath)) {
          return Util::Server::NotFound();
        }

        std::ofstream out(filePath);
        if (!out) {
          return Util::Server::Response{500, "text/plain",
                                        "Failed To Write File"};
        }

        out << req.body;
        return Util::Server::JSON("{\"success\": true}");
      });

  Util::Server::RegisterRoute(
      *session.server, "POST", "/api/project/file/create",
      [&session](const Util::Server::Request &req) {
        auto it = req.headers.find("X-File");
        if (it == req.headers.end()) {
          return Util::Server::Response{400, "text/plain", "Missing Header"};
        }

        std::filesystem::path requested(it->second);
        if (!IsTextProjectFile(requested)) {
          return Util::Server::Response{403, "text/plain",
                                        "Unsupported File Type"};
        }

        auto projectRoot = std::filesystem::absolute(session.projectRoot);
        auto sourceRoot = GetEditableProjectRoot(projectRoot);
        std::filesystem::path filePath;
        if (!ResolveProjectFilePath(sourceRoot, it->second, filePath)) {
          return Util::Server::Response{403, "text/plain", "Invalid Path"};
        }

        if (std::filesystem::exists(filePath)) {
          return Util::Server::Response{409, "text/plain", "File Exists"};
        }

        std::filesystem::create_directories(filePath.parent_path());
        std::ofstream out(filePath);
        if (!out) {
          return Util::Server::Response{500, "text/plain",
                                        "Failed To Create File"};
        }

        out << req.body;
        return Util::Server::JSON("{\"success\": true}");
      });

  Util::Server::RegisterRoute(
      *session.server, "POST", "/api/project/file/delete",
      [&session](const Util::Server::Request &req) {
        auto it = req.headers.find("X-File");
        if (it == req.headers.end()) {
          return Util::Server::Response{400, "text/plain", "Missing Header"};
        }

        auto projectRoot = std::filesystem::absolute(session.projectRoot);
        auto sourceRoot = GetEditableProjectRoot(projectRoot);
        std::filesystem::path filePath;
        if (!ResolveProjectFilePath(sourceRoot, it->second, filePath) ||
            !std::filesystem::exists(filePath) ||
            !std::filesystem::is_regular_file(filePath)) {
          return Util::Server::NotFound();
        }

        if (!IsTextProjectFile(filePath)) {
          return Util::Server::Response{403, "text/plain",
                                        "Unsupported File Type"};
        }

        std::filesystem::remove(filePath);
        PruneEmptyDirectories(sourceRoot, filePath.parent_path());
        return Util::Server::JSON("{\"success\": true}");
      });

  Util::Server::RegisterRoute(
      *session.server, "POST", "/api/editor/exit",
      [&session](const Util::Server::Request &req) {
        std::exit(0);
        return Util::Server::JSON("{\"success\": true}");
      });

  Util::Server::RegisterRoute(
      *session.server, "GET", "/api/theme/files",
      [&session](const Util::Server::Request &req) {
        auto themeDir = std::filesystem::absolute(session.projectRoot) /
                        "content/assets/theme";
        if (!std::filesystem::exists(themeDir)) {
          return Util::Server::JSON("[]");
        }

        std::string json = "[";
        bool first = true;
        for (const auto &entry :
             std::filesystem::recursive_directory_iterator(themeDir)) {
          if (entry.is_regular_file() && entry.path().extension() == ".css") {
            if (!first) {
              json += ",";
            }
            json += "\"" + entry.path().filename().string() + "\"";
            first = false;
          }
        }
        json += "]";
        return Util::Server::JSON(json);
      });

  Util::Server::RegisterRoute(
      *session.server, "GET", "/api/theme/read",
      [&session](const Util::Server::Request &req) {
        auto it = req.headers.find("X-File");
        if (it == req.headers.end()) {
          return Util::Server::Response{400, "text/plain", "Missing Header"};
        }

        auto filePath = std::filesystem::absolute(session.projectRoot) /
                        "content/assets/theme" / it->second;
        if (std::filesystem::exists(filePath)) {
          return Util::Server::File(filePath.string(), "text/css");
        }
        return Util::Server::NotFound();
      });

  Util::Server::RegisterRoute(
      *session.server, "POST", "/api/theme/write",
      [&session](const Util::Server::Request &req) {
        auto it = req.headers.find("X-File");
        if (it == req.headers.end()) {
          return Util::Server::Response{400, "text/plain", "Missing Header"};
        }
        if (req.body.empty()) {
          return Util::Server::Response{400, "text/plain", "Empty Body"};
        }

        auto filePath = std::filesystem::absolute(session.projectRoot) /
                        "content/assets/theme" / it->second;
        std::ofstream out(filePath);
        out << req.body;
        return Util::Server::JSON("{\"success\": true}");
      });

  Util::Server::Start(*session.server);
  session.isRunning = true;

  std::string startUrl =
      "http://localhost:" + std::to_string(session.port) + "/";
  if (openBrowser) {
    OpenBrowser(startUrl);
  }

  Log::Info("Editor launched at " + startUrl);
  return true;
}

auto Stop(Session &session) -> void {
  if (session.isRunning && session.server) {
    Util::Server::Stop(*session.server);
    session.isRunning = false;
    session.server = nullptr;
  }
}

auto IsRunning(const Session &session) -> bool { return session.isRunning; }

auto OpenBrowser(const std::string &url) -> void {
  std::string command;
#ifdef _WIN32
  command = "start " + url;
#elif __linux__
  command = "xdg-open " + url + " > /dev/null 2>&1 &";
#else
  command = "open " + url;
#endif
  std::system(command.c_str());
}

} // namespace Editor
