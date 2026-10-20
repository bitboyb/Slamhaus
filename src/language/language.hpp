#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

namespace Language {

enum class Severity : std::uint8_t { Warning, Error };

struct Position {
  std::size_t line = 0;
  std::size_t character = 0;
};

struct Range {
  Position start;
  Position end;
};

struct Diagnostic {
  std::filesystem::path path;
  Severity severity = Severity::Error;
  std::string code;
  std::string message;
  std::string help;
  Range range;
};

struct Runtime {
  std::string module;
  std::string application;
  std::string canvas;
};

struct Metadata {
  std::string siteTitle;
  std::string siteDescription;
  std::string siteUrl;
  std::string favicon;
  std::string pageTitle;
  std::string pageDescription;
  std::string keywords;
  std::string date;
  std::vector<std::string> tags;
  std::string canonical;
  std::string permalink;
  Runtime runtime;
  bool draft = false;
  bool noindex = false;
  bool toc = false;
};

struct Binding {
  std::string event;
  std::string function;
  std::vector<std::string> arguments;
  std::string output;
};

using Properties = std::map<std::string, std::string>;

enum class NodeKind : std::uint8_t { Markdown, Component };

struct Node {
  NodeKind kind = NodeKind::Markdown;
  std::string name;
  std::string text;
  std::string id;
  Properties properties;
  Binding binding;
  std::vector<Node> children;
  Range range;
};

struct Behavior {
  std::string language;
  std::string source;
  Range range;
  std::vector<std::string> exports;
};

struct Document {
  std::filesystem::path path;
  Metadata metadata;
  std::vector<Node> nodes;
  Behavior behavior;
  std::vector<Diagnostic> diagnostics;
};

struct RenderResult {
  std::string html;
  std::vector<Diagnostic> diagnostics;
};

struct RenderOptions {
  bool unsafeHtml = false;
  std::function<std::string(const std::string &)> resolveUrl;
  std::string runtimeUrl = "/slamhaus-runtime.js";
  std::string moduleUrl;
};

struct SemanticToken {
  Range range;
  std::string type;
};

auto Parse(const std::string &source, const std::filesystem::path &path = {},
           Metadata defaults = {}) -> Document;
auto Render(const Document &document, const RenderOptions &options = {})
    -> RenderResult;
auto Format(const std::string &source) -> std::string;
auto SemanticTokens(const std::string &source, const std::filesystem::path &path = {})
    -> std::vector<SemanticToken>;
auto HasErrors(const std::vector<Diagnostic> &diagnostics) -> bool;
auto DiagnosticsJson(const std::vector<Diagnostic> &diagnostics) -> nlohmann::json;
auto SchemaJson() -> nlohmann::json;
auto EscapeHtml(const std::string &value) -> std::string;
auto AdjustLink(const std::string &value) -> std::string;

} // namespace Language
