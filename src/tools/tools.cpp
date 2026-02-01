#include "tools.hpp"
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
    return "text/plain"; // Served as raw text for debug or managed explicitly
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

  std::filesystem::path toolRoot = std::filesystem::absolute(spec.assetPath);
  std::filesystem::path distRoot =
      std::filesystem::absolute(projectRoot) / "output";

  // 1. Serve Tool Assets
  auto toolPrefix = "/tools/" + spec.id + "/";
  Util::Server::RegisterRoute(
      *session.server, "GET", toolPrefix,
      [toolRoot, toolPrefix](const Util::Server::Request &req) {
        std::string subPath = req.path.substr(toolPrefix.length());
        if (subPath.find("..") != std::string::npos)
          return Util::Server::NotFound();

        // Handle "editor.html" request by serving "editor.slam" or the actual
        // .html if present As per user request, Slamhaus should
        // generate/compile it. For now, if we cannot trigger a build easily, we
        // will serve the raw files OR rely on the fact that we might have
        // pre-built? The user said "you can compile the editor with this
        // workflow" but then it failed. We will stick to serving files for now,
        // but ensure we server editor.html even if it maps to editor.html shim.

        return Util::Server::File((toolRoot / subPath).string(),
                                  GetMimeType(subPath));
      });

  // 2. Serve Project Output
  Util::Server::RegisterRoute(*session.server, "GET", "/preview/",
                              [distRoot](const Util::Server::Request &req) {
                                std::string subPath = req.path.substr(9);
                                if (subPath.empty())
                                  subPath = "index.html";
                                if (subPath.find("..") != std::string::npos)
                                  return Util::Server::NotFound();
                                return Util::Server::File(
                                    (distRoot / subPath).string(),
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
