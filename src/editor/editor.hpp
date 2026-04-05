#pragma once

#ifndef EDITOR_HPP
#define EDITOR_HPP

#include <memory>
#include <string>

namespace Util::Server {
struct ServerContext;
}

namespace Editor {
/// Runtime session state for the separate editor frontend server.
struct Session {
  std::string projectRoot;
  int port;
  bool isRunning;
  std::shared_ptr<Util::Server::ServerContext> server;

  Session();
  ~Session();
};

/// Launches the separate editor frontend for the given project.
///
/// @param session The session object to populate
/// @param projectRoot The root directory of the project to edit
/// @param port The localhost port to bind the editor server to
/// @param openBrowser Whether to open the editor URL in the default browser
/// @return True if the editor launched successfully
auto Launch(Session &session, const std::string &projectRoot, int port = 8090,
            bool openBrowser = true) -> bool;

/// Stops a running editor session.
auto Stop(Session &session) -> void;

/// Checks whether an editor session is currently active.
auto IsRunning(const Session &session) -> bool;

/// Opens a URL in the system default browser.
auto OpenBrowser(const std::string &url) -> void;
} // namespace Editor

#endif // EDITOR_HPP
