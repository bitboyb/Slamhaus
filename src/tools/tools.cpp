#include "tools.hpp"
#include "../cli/cli.hpp"
#include "../core/generator.hpp"
#include "../util/log.hpp"
#include "../util/server.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace Tools {

namespace {
// Helper to map extensions to mime types
std::string GetMimeType(const std::string &path) {
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
  if (ext == ".jpg")
    return "image/jpeg";
  if (ext == ".wasm")
    return "application/wasm";
  if (ext == ".slam")
    return "text/plain";
  return "application/octet-stream";
}

// Hardcoded registry removed.
} // namespace

ToolSession::ToolSession() : port(0), isRunning(false), server(nullptr) {}

ToolSession::~ToolSession() {
  if (isRunning)
    StopTool(*this);
}

// GetRegistry and FindToolSpec removed.

bool LaunchTool(ToolSession &session, const std::string &projectRoot) {

  session.toolId = "system";
  session.projectRoot = projectRoot; // Default to current dir
  session.port = 8090;
  session.server = std::make_shared<Util::Server::ServerContext>(session.port);

  // System Paths
  std::filesystem::path systemSource =
      std::filesystem::absolute("tools/system/content");
  std::filesystem::path systemDist = std::filesystem::absolute("dist/system");

  // Clean build
  if (std::filesystem::exists(systemDist)) {
    std::filesystem::remove_all(systemDist);
  }
  std::filesystem::create_directories(systemSource / "assets");

  // Build System Site
  Log::Info("Building Dashboard from " + systemSource.string());
  if (!Generator::GenerateSite(systemSource, systemDist, true)) {
    Log::Error("Failed to build Dashboard.");
    return false;
  }

  // Explicitly copy assets (CSS/JS) because Generator might skip them on some
  // systems
  auto assetsSrc = systemSource / "assets";
  auto assetsDist = systemDist / "assets";
  if (std::filesystem::exists(assetsSrc)) {
    try {
      std::filesystem::copy(
          assetsSrc, assetsDist,
          std::filesystem::copy_options::recursive |
              std::filesystem::copy_options::overwrite_existing);
    } catch (std::exception &e) {
      Log::Error("Failed to copy system assets: " + std::string(e.what()));
    }
  }

  // 1. Serve System Site (Dashboard)
  // Serve everything in dist/system at root /
  Util::Server::RegisterRoute(
      *session.server, "GET", "/",
      [systemDist](const Util::Server::Request &req) {
        std::string subPath = req.path;
        if (subPath == "/")
          subPath = "/index.html";

        if (subPath.find("..") != std::string::npos)
          return Util::Server::NotFound();

        // Check file exists in system dist
        auto path = systemDist / subPath.substr(1); // remove leading /
        if (std::filesystem::exists(path)) {
          return Util::Server::File(path.string(), GetMimeType(path.string()));
        }
        return Util::Server::NotFound();
      });

  // 2. Project Preview (Hot Linked)
  Util::Server::RegisterRoute(
      *session.server, "GET", "/preview/",
      [&session](const Util::Server::Request &req) {
        std::string subPath = req.path.substr(9); // remove /preview/
        if (subPath.empty())
          subPath = "index.html";

        auto projRoot = std::filesystem::absolute(session.projectRoot);
        auto projContent = projRoot / "content";
        auto projDist = projRoot / "output";

        // Hot Link Theme
        if (subPath.find("assets/theme/") == 0) {
          std::string filename = subPath.substr(13);
          auto sourcePath = projContent / "assets/theme" / filename;
          if (std::filesystem::exists(sourcePath)) {
            return Util::Server::File(sourcePath.string(), "text/css");
          }
        }

        auto distPath = projDist / subPath;
        if (std::filesystem::exists(distPath)) {
          return Util::Server::File(distPath.string(), GetMimeType(subPath));
        }
        return Util::Server::NotFound();
      });

  // 3. API: Project Status
  Util::Server::RegisterRoute(*session.server, "GET", "/api/project/status",
                              [&session](const Util::Server::Request &req) {
                                return Util::Server::JSON("{\"path\": \"" +
                                                          session.projectRoot +
                                                          "\"}");
                              });

  // 4. API: Set Project Root
  Util::Server::RegisterRoute(
      *session.server, "POST", "/api/project/set-root",
      [&session](const Util::Server::Request &req) {
        if (req.body.empty())
          return Util::Server::Response{400, "text/plain", "Empty Path"};
        std::string newPath = req.body;
        if (std::filesystem::exists(newPath) &&
            std::filesystem::is_directory(newPath)) {
          session.projectRoot = newPath;
          Log::Info("Project root switched to: " + newPath);
          return Util::Server::JSON("{\"success\": true}");
        }
        return Util::Server::Response{400, "text/plain", "Invalid Directory"};
      });

  // 5. API: Build Project
  Util::Server::RegisterRoute(
      *session.server, "POST",
      "/api/project/build", // Renamed from /api/system/build
      [&session](
          const Util::Server::Request &req) { // Renamed from /api/system/build
        // Backward compatibility for editor.js which calls /api/system/build
        // ACTUALLY, I should register BOTH or update editor.js.
        // Let's execute logic.
        auto contentDir =
            std::filesystem::absolute(session.projectRoot) / "content";
        auto outputDir =
            std::filesystem::absolute(session.projectRoot) / "output";

        if (Generator::GenerateSite(contentDir, outputDir, true, true)) {
          std::cout << "\n[Dashboard] Project built successfully.\n";
          // We don't reprompt CLI menu here because we are in "Dashboard Mode"
          // But if we want to keep CLI interaction alive, we could.
          return Util::Server::JSON("{\"success\": true}");
        }
        return Util::Server::Response{500, "text/plain", "Build Failed"};
      });

  // ALIAS /api/system/build -> /api/project/build for Theme Editor
  // compatibility
  Util::Server::RegisterRoute(
      *session.server, "POST", "/api/system/build",
      [&session](const Util::Server::Request &req) {
        auto contentDir =
            std::filesystem::absolute(session.projectRoot) / "content";
        auto outputDir =
            std::filesystem::absolute(session.projectRoot) / "output";
        if (Generator::GenerateSite(contentDir, outputDir, true, true)) {
          return Util::Server::JSON("{\"success\": true}");
        }
        return Util::Server::Response{500, "text/plain", "Build Failed"};
      });

  // 6. API: System Exit
  Util::Server::RegisterRoute(
      *session.server, "POST", "/api/system/exit",
      [&session](const Util::Server::Request &req) {
        std::cout << "\n[Dashboard] Shutdown requested.\n";
        std::exit(0); // Aggressive exit? Or callback?
        // Ideally we set session.isRunning = false; but we need to break the
        // CLI loop. Exit(0) works for now.
        return Util::Server::JSON("{\"success\": true}");
      });

  // 7. Theme Editor APIs (Read/Write/Files)
  // Re-implemented to use session.projectRoot dynamically
  Util::Server::RegisterRoute(
      *session.server, "GET", "/api/theme/files",
      [&session](const Util::Server::Request &req) {
        auto themeDir = std::filesystem::absolute(session.projectRoot) /
                        "content/assets/theme";
        if (!std::filesystem::exists(themeDir))
          return Util::Server::JSON("[]");
        std::string json = "[";
        bool first = true;
        for (const auto &entry :
             std::filesystem::recursive_directory_iterator(themeDir)) {
          if (entry.is_regular_file() && entry.path().extension() == ".css") {
            if (!first)
              json += ",";
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
        if (it == req.headers.end())
          return Util::Server::Response{400, "text/plain", "Missing Header"};
        auto filePath = std::filesystem::absolute(session.projectRoot) /
                        "content/assets/theme" / it->second;
        if (std::filesystem::exists(filePath))
          return Util::Server::File(filePath.string(), "text/css");
        return Util::Server::NotFound();
      });

  Util::Server::RegisterRoute(
      *session.server, "POST", "/api/theme/write",
      [&session](const Util::Server::Request &req) {
        auto it = req.headers.find("X-File");
        if (it == req.headers.end())
          return Util::Server::Response{400, "text/plain", "Missing Header"};
        if (req.body.empty())
          return Util::Server::Response{400, "text/plain", "Empty Body"};
        auto filePath = std::filesystem::absolute(session.projectRoot) /
                        "content/assets/theme" / it->second;
        std::ofstream out(filePath);
        out << req.body;
        return Util::Server::JSON("{\"success\": true}");
      });

  Util::Server::Start(*session.server);
  session.isRunning = true;

  std::string startUrl = "http://localhost:8090/";
  OpenBrowser(startUrl);

  Log::Info("Dashboard launched at " + startUrl);
  return true;
}

void StopTool(ToolSession &session) {
  if (session.isRunning && session.server) {
    Util::Server::Stop(*session.server);
    session.isRunning = false;
    session.server = nullptr;
  }
}

bool IsToolRunning(const ToolSession &session) { return session.isRunning; }

void OpenBrowser(const std::string &url) {
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

} // namespace Tools
