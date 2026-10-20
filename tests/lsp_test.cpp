#include "lsp/lsp.hpp"

#include <catch2/catch_test_macros.hpp>

#include <sstream>

#include <nlohmann/json.hpp>

namespace {

auto Frame(const nlohmann::json &message) -> std::string {
  const auto body = message.dump();
  return "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body;
}

} // namespace

TEST_CASE("lsp advertises the enhanced language surface") {
  const std::string uri = "file:///tmp/test.slam";
  const std::string source = "# Page\n```slam-ui\nWidget Unknown();\n```\n"
                             "```cpp wasm\nauto Run() -> void {}\n```\n";
  std::ostringstream requests;
  requests << Frame({{"jsonrpc", "2.0"},
                     {"id", 1},
                     {"method", "initialize"},
                     {"params", nlohmann::json::object()}})
           << Frame({{"jsonrpc", "2.0"},
                     {"method", "textDocument/didOpen"},
                     {"params", {{"textDocument", {{"uri", uri}, {"text", source}}}}}})
           << Frame({{"jsonrpc", "2.0"},
                     {"id", 2},
                     {"method", "textDocument/completion"},
                     {"params", nlohmann::json::object()}})
           << Frame({{"jsonrpc", "2.0"},
                     {"id", 3},
                     {"method", "textDocument/semanticTokens/full"},
                     {"params", {{"textDocument", {{"uri", uri}}}}}})
           << Frame({{"jsonrpc", "2.0"}, {"id", 4}, {"method", "shutdown"}})
           << Frame({{"jsonrpc", "2.0"}, {"method", "exit"}});
  std::istringstream input(requests.str());
  std::ostringstream output;
  REQUIRE(Lsp::Run(input, output) == 0);
  REQUIRE(output.str().find("semanticTokensProvider") != std::string::npos);
  REQUIRE(output.str().find("SH1103") != std::string::npos);
  REQUIRE(output.str().find("Button") != std::string::npos);
  REQUIRE(output.str().find("\"data\"") != std::string::npos);
}

TEST_CASE("lsp publishes clean diagnostics for valid mixed source") {
  const std::string uri = "file:///tmp/valid.slam";
  const std::string source = "```slam-ui\nButton Go(\"Go\") => Run();\n```\n"
                             "```cpp wasm\nauto Run() -> void {}\n```\n";
  std::ostringstream requests;
  requests << Frame({{"jsonrpc", "2.0"}, {"id", 1}, {"method", "initialize"}})
           << Frame({{"jsonrpc", "2.0"},
                     {"method", "textDocument/didOpen"},
                     {"params", {{"textDocument", {{"uri", uri}, {"text", source}}}}}})
           << Frame({{"jsonrpc", "2.0"}, {"id", 2}, {"method", "shutdown"}})
           << Frame({{"jsonrpc", "2.0"}, {"method", "exit"}});
  std::istringstream input(requests.str());
  std::ostringstream output;
  REQUIRE(Lsp::Run(input, output) == 0);
  REQUIRE(output.str().find("\"diagnostics\":[]") != std::string::npos);
}

TEST_CASE("lsp document symbols include markdown headings") {
  const std::string uri = "file:///tmp/symbols.slam";
  std::ostringstream requests;
  requests << Frame({{"jsonrpc", "2.0"}, {"id", 1}, {"method", "initialize"}})
           << Frame({{"jsonrpc", "2.0"},
                     {"method", "textDocument/didOpen"},
                     {"params",
                      {{"textDocument", {{"uri", uri}, {"text", "# Inventory\n"}}}}}})
           << Frame({{"jsonrpc", "2.0"},
                     {"id", 2},
                     {"method", "textDocument/documentSymbol"},
                     {"params", {{"textDocument", {{"uri", uri}}}}}})
           << Frame({{"jsonrpc", "2.0"}, {"id", 3}, {"method", "shutdown"}})
           << Frame({{"jsonrpc", "2.0"}, {"method", "exit"}});
  std::istringstream input(requests.str());
  std::ostringstream output;
  REQUIRE(Lsp::Run(input, output) == 0);
  REQUIRE(output.str().find("Inventory") != std::string::npos);
}

TEST_CASE("lsp formatting is conservative and idempotent") {
  const std::string uri = "file:///tmp/format.slam";
  const std::string source = "```slam-ui\nButton Go(\"Go\");\n```\n";
  std::ostringstream requests;
  requests << Frame({{"jsonrpc", "2.0"}, {"id", 1}, {"method", "initialize"}})
           << Frame({{"jsonrpc", "2.0"},
                     {"method", "textDocument/didOpen"},
                     {"params", {{"textDocument", {{"uri", uri}, {"text", source}}}}}})
           << Frame({{"jsonrpc", "2.0"},
                     {"id", 2},
                     {"method", "textDocument/formatting"},
                     {"params", {{"textDocument", {{"uri", uri}}}}}})
           << Frame({{"jsonrpc", "2.0"}, {"id", 3}, {"method", "shutdown"}})
           << Frame({{"jsonrpc", "2.0"}, {"method", "exit"}});
  std::istringstream input(requests.str());
  std::ostringstream output;
  REQUIRE(Lsp::Run(input, output) == 0);
  REQUIRE(output.str().find("Button Go") != std::string::npos);
  REQUIRE(output.str().find("documentFormattingProvider") != std::string::npos);
}

TEST_CASE("lsp exits unsuccessfully without shutdown") {
  std::istringstream input(Frame({{"jsonrpc", "2.0"}, {"method", "exit"}}));
  std::ostringstream output;
  REQUIRE(Lsp::Run(input, output) == 1);
}

TEST_CASE("lsp rejects invalid requests without throwing") {
  const auto invalid =
      Frame({{"jsonrpc", "2.0"}, {"id", 9}, {"method", "textDocument/didOpen"}});
  const auto shutdown = Frame({{"jsonrpc", "2.0"}, {"id", 10}, {"method", "shutdown"}});
  const auto exit = Frame({{"jsonrpc", "2.0"}, {"method", "exit"}});
  std::istringstream input(invalid + shutdown + exit);
  std::ostringstream output;
  REQUIRE_NOTHROW(Lsp::Run(input, output));
  REQUIRE(output.str().find("Invalid params") != std::string::npos);
}
