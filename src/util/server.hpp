#pragma once

#ifndef UTIL_SERVER_HPP
#define UTIL_SERVER_HPP

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <thread>

namespace Util::Server {

/// Simple HTTP Request structure.
struct Request {
  std::string method;
  std::string path;
  std::map<std::string, std::string> headers;
  std::string body;
};

/// Simple HTTP Response structure.
struct Response {
  int status = 200;
  std::string contentType = "text/plain";
  std::string body;
};

/// Function signature for handling requests.
using Handler = std::function<Response(const Request &)>;

/// Context for a running server.
struct ServerContext {
  int port;
  std::atomic<bool> running;
  std::shared_ptr<std::thread>
      worker; // Shared ptr to allow strict value semantics elsewhere
  std::map<std::string, Handler> routes;

  ServerContext(int p);
  ~ServerContext();
};

/// Starts the server in a background thread.
void Start(ServerContext &ctx);

/// Stops the server and joins the thread.
void Stop(ServerContext &ctx);

/// Registers a handler for a specific method and path prefix.
/// @param method The HTTP method (GET, POST, etc.)
/// @param path The path to match (exact or prefix).
/// @param handler The function to call when matched.
void RegisterRoute(ServerContext &ctx, const std::string &method,
                   const std::string &path, Handler handler);

/// Helper to create a 404 response.
Response NotFound();

/// Helper to create a JSON response.
Response JSON(const std::string &json);

/// Helper to create a file serving response.
Response File(const std::string &path, const std::string &contentType);

} // namespace Util::Server

#endif // UTIL_SERVER_HPP
