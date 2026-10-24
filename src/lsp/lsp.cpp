#include "lsp/lsp.hpp"

#include "language/language.hpp"
#include "version.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <filesystem>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace Lsp {
namespace {

using Json = nlohmann::json;

auto ReadMessage(std::istream &input) -> std::optional<Json> {
  std::string header;
  std::size_t length = 0;
  while (std::getline(input, header)) {
    if (!header.empty() && header.back() == '\r')
      header.pop_back();
    if (header.empty())
      break;
    constexpr auto prefix = "Content-Length:";
    if (header.compare(0, std::char_traits<char>::length(prefix), prefix) == 0) {
      const auto value = header.substr(std::char_traits<char>::length(prefix));
      const auto first = value.find_first_not_of(' ');
      if (first == std::string::npos)
        return std::nullopt;
      const char *begin = value.data() + first;
      const char *end = value.data() + value.size();
      const auto parsed = std::from_chars(begin, end, length);
      constexpr std::size_t maximumMessage = 16ULL * 1024ULL * 1024ULL;
      if (parsed.ec != std::errc{} || parsed.ptr != end || length > maximumMessage)
        return std::nullopt;
    }
  }
  if (length == 0)
    return std::nullopt;
  std::string body(length, '\0');
  input.read(body.data(), static_cast<std::streamsize>(length));
  if (input.gcount() != static_cast<std::streamsize>(length))
    return std::nullopt;
  return Json::parse(body, nullptr, false);
}

auto WriteMessage(std::ostream &output, const Json &message) -> void {
  const std::string body = message.dump();
  output << "Content-Length: " << body.size() << "\r\n\r\n" << body;
  output.flush();
}

auto DecodeUri(std::string uri) -> std::filesystem::path {
  constexpr auto prefix = "file://";
  if (uri.compare(0, std::char_traits<char>::length(prefix), prefix) == 0)
    uri.erase(0, std::char_traits<char>::length(prefix));
  std::string decoded;
  for (std::size_t i = 0; i < uri.size(); ++i) {
    if (uri[i] == '%' && i + 2 < uri.size()) {
      const auto hex = uri.substr(i + 1, 2);
      decoded.push_back(static_cast<char>(std::stoi(hex, nullptr, 16)));
      i += 2;
    } else {
      decoded.push_back(uri[i]);
    }
  }
#ifdef _WIN32
  if (decoded.size() > 2 && decoded.front() == '/' && decoded[2] == ':')
    decoded.erase(decoded.begin());
#endif
  return decoded;
}

auto LspRange(const Language::Range &range) -> Json {
  return {{"start", {{"line", range.start.line}, {"character", range.start.character}}},
          {"end", {{"line", range.end.line}, {"character", range.end.character}}}};
}

auto Publish(std::ostream &output, const std::string &uri, const std::string &source)
    -> void {
  const auto document = Language::Parse(source, DecodeUri(uri));
  const auto rendered = Language::Render(document);
  Json diagnostics = Json::array();
  for (const auto &item : rendered.diagnostics) {
    Json diagnostic = {{"range", LspRange(item.range)},
                       {"severity", item.severity == Language::Severity::Error ? 1 : 2},
                       {"code", item.code},
                       {"source", "slamhaus"},
                       {"message", item.message}};
    if (!item.help.empty())
      diagnostic["data"] = {{"help", item.help}};
    diagnostics.push_back(std::move(diagnostic));
  }
  WriteMessage(output, {{"jsonrpc", "2.0"},
                        {"method", "textDocument/publishDiagnostics"},
                        {"params", {{"uri", uri}, {"diagnostics", diagnostics}}}});
}

auto CompletionItems() -> Json {
  Json items = Json::array();
  const auto schema = Language::SchemaJson();
  for (const auto &component : schema["components"]) {
    const std::string name = component["name"];
    items.push_back({{"label", name},
                     {"kind", 7},
                     {"detail", "Slam UI component"},
                     {"insertText", name + " ${1:Name}($2) {\n\t$0\n}"},
                     {"insertTextFormat", 2}});
  }
  for (const auto *field : {"title", "description", "draft", "date", "tags",
                            "canonical", "permalink", "noindex", "toc", "runtime"})
    items.push_back({{"label", field}, {"kind", 10}, {"detail", "Page metadata"}});
  return items;
}

auto SemanticTokenData(const std::string &source, const std::filesystem::path &path)
    -> Json {
  static const std::vector<std::string> types = {
      "namespace",     "type",      "class",    "enum",     "interface",  "struct",
      "typeParameter", "parameter", "variable", "property", "enumMember", "event",
      "function",      "method",    "macro",    "keyword",  "modifier",   "comment",
      "string",        "number",    "regexp",   "operator", "decorator"};
  Json data = Json::array();
  std::size_t previousLine = 0;
  std::size_t previousCharacter = 0;
  for (const auto &token : Language::SemanticTokens(source, path)) {
    const auto found = std::find(types.begin(), types.end(), token.type);
    if (found == types.end() || token.range.start.line != token.range.end.line)
      continue;
    const auto deltaLine = token.range.start.line - previousLine;
    const auto deltaCharacter = deltaLine == 0
                                    ? token.range.start.character - previousCharacter
                                    : token.range.start.character;
    data.push_back(deltaLine);
    data.push_back(deltaCharacter);
    data.push_back(token.range.end.character - token.range.start.character);
    data.push_back(std::distance(types.begin(), found));
    data.push_back(0);
    previousLine = token.range.start.line;
    previousCharacter = token.range.start.character;
  }
  return {{"data", std::move(data)}};
}

auto Symbols(const std::string &source) -> Json {
  Json symbols = Json::array();
  std::istringstream input(source);
  std::string line;
  std::size_t number = 0;
  while (std::getline(input, line)) {
    std::size_t level = 0;
    while (level < line.size() && line[level] == '#')
      ++level;
    if (level >= 1 && level <= 6 && level < line.size() && line[level] == ' ') {
      const Language::Range range{{number, 0}, {number, line.size()}};
      symbols.push_back({{"name", line.substr(level + 1)},
                         {"kind", 15},
                         {"range", LspRange(range)},
                         {"selectionRange", LspRange(range)}});
    }
    ++number;
  }
  return symbols;
}

auto Response(const Json &request, Json result) -> Json {
  return {{"jsonrpc", "2.0"}, {"id", request.at("id")}, {"result", std::move(result)}};
}

} // namespace

auto Run(std::istream &input, std::ostream &output) -> int {
  std::map<std::string, std::string> documents;
  bool shutdown = false;
  while (const auto message = ReadMessage(input)) {
    if (message->is_discarded() || !message->contains("method"))
      continue;
    try {
      const std::string method = message->at("method");
      if (method == "initialize") {
        const Json tokenTypes = {"namespace", "type",     "class",         "enum",
                                 "interface", "struct",   "typeParameter", "parameter",
                                 "variable",  "property", "enumMember",    "event",
                                 "function",  "method",   "macro",         "keyword",
                                 "modifier",  "comment",  "string",        "number",
                                 "regexp",    "operator", "decorator"};
        WriteMessage(
            output,
            Response(
                *message,
                {{"capabilities",
                  {{"textDocumentSync", 1},
                   {"completionProvider", Json::object()},
                   {"hoverProvider", true},
                   {"documentSymbolProvider", true},
                   {"documentFormattingProvider", true},
                   {"semanticTokensProvider",
                    {{"legend",
                      {{"tokenTypes", tokenTypes}, {"tokenModifiers", Json::array()}}},
                     {"full", true}}}}},
                 {"serverInfo",
                  {{"name", "slamhaus"}, {"version", SLAMHAUS_VERSION}}}}));
      } else if (method == "initialized") {
        continue;
      } else if (method == "shutdown") {
        shutdown = true;
        WriteMessage(output, Response(*message, nullptr));
      } else if (method == "exit") {
        return shutdown ? 0 : 1;
      } else if (method == "textDocument/didOpen") {
        const auto &document = message->at("params").at("textDocument");
        const std::string uri = document.at("uri");
        documents[uri] = document.at("text");
        Publish(output, uri, documents[uri]);
      } else if (method == "textDocument/didChange") {
        const std::string uri = message->at("params").at("textDocument").at("uri");
        const auto &changes = message->at("params").at("contentChanges");
        if (!changes.empty())
          documents[uri] = changes.back().at("text");
        Publish(output, uri, documents[uri]);
      } else if (method == "textDocument/didClose") {
        const std::string uri = message->at("params").at("textDocument").at("uri");
        documents.erase(uri);
        WriteMessage(output,
                     {{"jsonrpc", "2.0"},
                      {"method", "textDocument/publishDiagnostics"},
                      {"params", {{"uri", uri}, {"diagnostics", Json::array()}}}});
      } else if (method == "textDocument/completion") {
        WriteMessage(output, Response(*message, CompletionItems()));
      } else if (method == "textDocument/hover") {
        WriteMessage(
            output,
            Response(*message,
                     {{"contents",
                       {{"kind", "markdown"},
                        {"value", std::string("Slamhaus ") + SLAMHAUS_VERSION +
                                      " Markdown, Slam UI, and C++ source"}}}}));
      } else if (method == "textDocument/documentSymbol") {
        const std::string uri = message->at("params").at("textDocument").at("uri");
        WriteMessage(output, Response(*message, Symbols(documents[uri])));
      } else if (method == "textDocument/formatting") {
        const std::string uri = message->at("params").at("textDocument").at("uri");
        const auto formatted = Language::Format(documents.at(uri));
        WriteMessage(
            output,
            Response(*message,
                     Json::array({{{"range",
                                    {{"start", {{"line", 0}, {"character", 0}}},
                                     {"end", {{"line", 1000000}, {"character", 0}}}}},
                                   {"newText", formatted}}})));
      } else if (method == "textDocument/semanticTokens/full") {
        const std::string uri = message->at("params").at("textDocument").at("uri");
        WriteMessage(output, Response(*message, SemanticTokenData(documents.at(uri),
                                                                  DecodeUri(uri))));
      } else if (message->contains("id")) {
        WriteMessage(output,
                     {{"jsonrpc", "2.0"},
                      {"id", message->at("id")},
                      {"error", {{"code", -32601}, {"message", "Method not found"}}}});
      }
    } catch (const std::exception &) {
      if (message->contains("id"))
        WriteMessage(output,
                     {{"jsonrpc", "2.0"},
                      {"id", message->at("id")},
                      {"error", {{"code", -32602}, {"message", "Invalid params"}}}});
    }
  }
  return shutdown ? 0 : 1;
}

} // namespace Lsp
