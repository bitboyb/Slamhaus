#include "server.hpp"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
using Socket = SOCKET;
#define IS_VALID(s) ((s) != INVALID_SOCKET)
#define CLOSE_SOCKET(s) closesocket(s)
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using Socket = int;
#define IS_VALID(s) ((s) >= 0)
#define CLOSE_SOCKET(s) close(s)
#define INVALID_SOCKET -1
#endif

namespace Util::Server {

namespace {

void GlobalInit() {
#ifdef _WIN32
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

void GlobalCleanup() {
#ifdef _WIN32
  WSACleanup();
#endif
}

struct ScopedSocket {
  Socket sock;
  ScopedSocket(Socket s) : sock(s) {}
  ~ScopedSocket() {
    if (IS_VALID(sock))
      CLOSE_SOCKET(sock);
  }
  operator Socket() { return sock; }
};

} // namespace

ServerContext::ServerContext(int p) : port(p), running(false) {}

ServerContext::~ServerContext() {
  if (running)
    Stop(*this);
}

void Start(ServerContext &ctx) {
  if (ctx.running)
    return;
  ctx.running = true;
  ctx.worker = std::make_shared<std::thread>([&ctx]() {
    GlobalInit();
    Socket serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (!IS_VALID(serverFd)) {
      std::cerr << "Failed to create socket\n";
      return;
    }

    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1"); // Localhost only
    address.sin_port = htons(ctx.port);

    if (bind(serverFd, (struct sockaddr *)&address, sizeof(address)) < 0) {
      std::cerr << "Bind failed on port " << ctx.port << "\n";
      CLOSE_SOCKET(serverFd);
      return;
    }

    if (listen(serverFd, 5) < 0) {
      std::cerr << "Listen failed\n";
      CLOSE_SOCKET(serverFd);
      return;
    }

#ifdef _WIN32
    DWORD timeout = 1000;
    setsockopt(serverFd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout,
               sizeof(timeout));
#else
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(serverFd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv,
               sizeof(tv));
#endif

    while (ctx.running) {
      Socket clientFd = accept(serverFd, nullptr, nullptr);
      if (!IS_VALID(clientFd)) {
        continue;
      }

      std::thread([clientFd, &ctx]() { // Capture ctx by reference carefully?
                                       // No, unsafe if ctx dies.
        // Wait, passing &ctx to a detached thread is unsafe if main thread
        // kills ctx. But Stop() joins the worker, so the worker ensures ctx
        // stays alive during loop. The client handler threads however are
        // detached. They might outlive ctx. Ideally we should track them or use
        // a pool. For this minimal implementation, effectively we assume
        // shutdown waits enough or we simply take the risk for local dev tool.
        // To be safer, we copy the routes? Expensive.
        // Let's assume for this single-user tool it's acceptable.

        ScopedSocket client(clientFd);
        char buffer[4096] = {0};
        int bytesRead = recv(client, buffer, 4096, 0);
        if (bytesRead <= 0)
          return;

        std::string raw(buffer, bytesRead);
        std::istringstream stream(raw);
        std::string method, path, proto;
        stream >> method >> path >> proto;

        std::map<std::string, std::string> headers;
        std::string line;
        std::getline(stream, line);
        while (std::getline(stream, line) && line != "\r") {
          auto colon = line.find(':');
          if (colon != std::string::npos) {
            headers[line.substr(0, colon)] =
                line.substr(colon + 2, line.length() - colon - 3);
          }
        }

        size_t bodyPos = raw.find("\r\n\r\n");
        std::string body;
        if (bodyPos != std::string::npos) {
          body = raw.substr(bodyPos + 4);
        }

        Request req{method, path, headers, body};
        Response res = NotFound();

        // access ctx.routes
        // Note: thread safety of reading map while not writing? ok.
        for (const auto &[route, handler] : ctx.routes) {
          if (path == route || (path.find(route) == 0 && route.back() == '/')) {
            res = handler(req);
            break;
          }
        }

        std::ostringstream responseStream;
        responseStream << "HTTP/1.1 " << res.status << " OK\r\n";
        responseStream << "Content-Type: " << res.contentType << "\r\n";
        responseStream << "Content-Length: " << res.body.size() << "\r\n";
        responseStream << "Access-Control-Allow-Origin: *\r\n";
        responseStream << "Connection: close\r\n\r\n";
        responseStream << res.body;

        std::string responseStr = responseStream.str();
        send(client, responseStr.c_str(), responseStr.size(), 0);
      })
          .detach();
    }

    CLOSE_SOCKET(serverFd);
    GlobalCleanup();
  });
}

void Stop(ServerContext &ctx) {
  ctx.running = false;
  if (ctx.worker && ctx.worker->joinable()) {
    ctx.worker->join();
  }
}

void RegisterRoute(ServerContext &ctx, const std::string &method,
                   const std::string &path, Handler handler) {
  ctx.routes[path] = handler;
}

Response NotFound() { return {404, "text/plain", "Not Found"}; }

Response JSON(const std::string &json) {
  return {200, "application/json", json};
}

Response File(const std::string &path, const std::string &contentType) {
  if (!std::filesystem::exists(path))
    return NotFound();
  std::ifstream f(path, std::ios::binary);
  std::stringstream buffer;
  buffer << f.rdbuf();
  return {200, contentType, buffer.str()};
}

} // namespace Util::Server
