#include "tools.hpp"
#include "../core/generator.hpp"
#include "../util/json.hpp"
#include "../util/log.hpp"
#include "../util/server.hpp"
#include <cstdlib>
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

// Hardcoded registry for now, could be dynamic later
std::vector<ToolSpec> registry = {
    {"theme-editor", "Theme Editor", "tools/theme-editor", "editor.slam"}};
} // namespace

ToolSession::ToolSession() : port(0), isRunning(false), server(nullptr) {}

ToolSession::~ToolSession() {
  if (isRunning)
    StopTool(*this);
}

std::vector<ToolSpec> GetRegistry() { return registry; }

const ToolSpec *FindToolSpec(const std::string &id) {
  for (const auto &spec : registry) {
    if (spec.id == id)
      return &spec;
  }
  return nullptr;
}

bool LaunchTool(ToolSession &session, const ToolSpec &spec,
                const std::string &projectRoot) {
  if (session.isRunning) {
    StopTool(session);
  }

  session.toolId = spec.id;
  session.projectRoot = projectRoot;
  session.port = 8090;
  session.server = std::make_shared<Util::Server::ServerContext>(session.port);

  std::filesystem::path toolSource = std::filesystem::absolute(spec.assetPath);
  std::filesystem::path toolDist = toolSource / "dist";
  std::filesystem::path projectDist =
      std::filesystem::absolute(projectRoot) / "output";

  // Build the tool itself!
  Log::Info("Building tool assets from " + toolSource.string());

  // Clean output first to avoid interactive prompt in Generator::GenerateSite
  if (std::filesystem::exists(toolDist)) {
    std::filesystem::remove_all(toolDist);
  }

  // Ensure assets directory exists to satisfy Generator strictness
  std::filesystem::create_directories(toolSource / "assets/theme");

  // Copy Project Theme to Tool Assets (as requested by user)
  std::filesystem::path projectTheme =
      std::filesystem::absolute(projectRoot) / "content/assets/theme";
  std::filesystem::path toolTheme = toolSource / "assets/theme";

  if (std::filesystem::exists(projectTheme)) {
    try {
      // Copy options: recursive, overwrite
      std::filesystem::copy(
          projectTheme, toolTheme,
          std::filesystem::copy_options::recursive |
              std::filesystem::copy_options::overwrite_existing);
    } catch (std::exception &e) {
      Log::Error("Failed to copy project theme to editor: " +
                 std::string(e.what()));
    }
  }

  // Ensure we have a minimal environment for the generator if needed.
  // The generator might expect the destination to be empty or valid.
  // We'll trust it works.
  if (!Generator::GenerateSite(toolSource, toolDist)) {
    Log::Error("Failed to build tool assets.");
    return false;
  }

  // 1. Serve built Tool Assets
  auto toolPrefix = "/tools/" + spec.id + "/";
  Util::Server::RegisterRoute(
      *session.server, "GET", toolPrefix,
      [toolDist, toolPrefix](const Util::Server::Request &req) {
        std::string subPath = req.path.substr(toolPrefix.length());

        // Default entry
        if (subPath.empty())
          subPath = "index.html";

        // Map editor.slam request to editor.html which exists in dist now
        if (subPath == "editor.slam")
          subPath = "editor.html";

        if (subPath.find("..") != std::string::npos)
          return Util::Server::NotFound();

        return Util::Server::File((toolDist / subPath).string(),
                                  GetMimeType(subPath));
      });

  // 2. Serve Project Output
  Util::Server::RegisterRoute(*session.server, "GET", "/preview/",
                              [projectDist](const Util::Server::Request &req) {
                                std::string subPath = req.path.substr(9);
                                if (subPath.empty())
                                  subPath = "index.html";
                                if (subPath.find("..") != std::string::npos)
                                  return Util::Server::NotFound();
                                return Util::Server::File(
                                    (projectDist / subPath).string(),
                                    GetMimeType(subPath));
                              });

  // 3. API: Theme
  auto themePath =
      std::filesystem::absolute(projectRoot) / "content/style/theme.json";

  // GET
  Util::Server::RegisterRoute(
      *session.server, "GET", "/api/theme",
      [themePath](const Util::Server::Request &req) {
        if (std::filesystem::exists(themePath)) {
          return Util::Server::File(themePath.string(), "application/json");
        }
        return Util::Server::JSON(
            "{\"primary_color\": \"#ff0000\", \"font_size\": \"16px\"}");
      });

  // POST
  Util::Server::RegisterRoute(
      *session.server, "POST", "/api/theme",
      [themePath](const Util::Server::Request &req) {
        auto node = Util::JSON::Parse(req.body);
        if (Util::JSON::IsNull(node))
          return Util::Server::Response{400, "text/plain", "Invalid JSON"};

        std::filesystem::create_directories(themePath.parent_path());
        std::ofstream out(themePath);
        out << Util::JSON::Stringify(node, true);
        return Util::Server::JSON("{\"success\": true}");
      });

  Util::Server::Start(*session.server);
  session.isRunning = true;

  // We point to editor.html (which is what editor.slam compiles to)
  std::string startUrl = "http://localhost:8090" + toolPrefix + "editor.html";
  OpenBrowser(startUrl);

  Log::Info("Tool launched at " + startUrl);
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
  command = "xdg-open " + url;
#else
  command = "open " + url;
#endif
  std::system(command.c_str());
}

} // namespace Tools
