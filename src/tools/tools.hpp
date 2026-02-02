#pragma once

#ifndef TOOLS_TOOLS_HPP
#define TOOLS_TOOLS_HPP

#include <memory>
#include <string>
#include <vector>

namespace Util::Server {
struct ServerContext;
}

namespace Tools {

/// Runtime session state for a running tool.
struct ToolSession {
  std::string toolId;
  int port;
  std::string projectRoot;
  bool isRunning;
  std::shared_ptr<Util::Server::ServerContext> server;

  ToolSession();
  ~ToolSession();
};

/// Launches the Unified System Dashboard.
/// @param session The session object to populate.
/// @param projectRoot The root directory of the user's project.
/// @return true if launched successfully.
bool LaunchTool(ToolSession &session, const std::string &projectRoot);

/// Stops a running tool session.
void StopTool(ToolSession &session);

/// Checks if a tool session is currently active.
bool IsToolRunning(const ToolSession &session);

/// Opens a URL in the system default browser.
void OpenBrowser(const std::string &url);

} // namespace Tools

#endif // TOOLS_TOOLS_HPP
