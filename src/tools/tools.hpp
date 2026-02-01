#pragma once

#ifndef TOOLS_TOOLS_HPP
#define TOOLS_TOOLS_HPP

#include <memory>
#include <string>
#include <vector>

namespace Util::Server {
struct Server;
}

namespace Tools {

/// Specification for a tool that can be launched.
struct ToolSpec {
  std::string id;        ///< Unique identifier (e.g. "theme-editor")
  std::string name;      ///< Display name (e.g. "Theme Editor")
  std::string assetPath; ///< Path to assets relative to repo root (e.g.
                         ///< "tools/theme-editor")
  std::string entryFile; ///< Entry point file (e.g. "editor.slam")
};

/// Runtime session state for a running tool.
struct ToolSession {
  std::string toolId;
  int port;
  std::string projectRoot;
  bool isRunning;
  std::shared_ptr<Util::Server::Server> server;

  ToolSession();
  ~ToolSession();
};

/// Returns a list of all available tools.
std::vector<ToolSpec> GetRegistry();

/// Finds a tool specification by ID.
/// @return Pointer to spec or nullptr if not found.
const ToolSpec *FindToolSpec(const std::string &id);

/// Launches a tool session.
/// @param session The session object to populate.
/// @param spec The tool specification.
/// @param projectRoot The root directory of the user's project.
/// @return true if launched successfully.
bool LaunchTool(ToolSession &session, const ToolSpec &spec,
                const std::string &projectRoot);

/// Stops a running tool session.
void StopTool(ToolSession &session);

/// Checks if a tool session is currently active.
bool IsToolRunning(const ToolSession &session);

/// Opens a URL in the system default browser.
void OpenBrowser(const std::string &url);

} // namespace Tools

#endif // TOOLS_TOOLS_HPP
