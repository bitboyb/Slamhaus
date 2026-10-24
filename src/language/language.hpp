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

/** Severity assigned to a user-facing diagnostic. */
enum class Severity : std::uint8_t { Warning, Error };

/** Zero-based source position used by diagnostics and editor features. */
struct Position {
  std::size_t line = 0;
  std::size_t character = 0;
};

/** Half-open source range. Lines and characters are zero based. */
struct Range {
  Position start;
  Position end;
};

/** A parser, validator, or renderer message associated with a source file. */
struct Diagnostic {
  std::filesystem::path path;
  Severity severity = Severity::Error;
  std::string code;
  std::string message;
  std::string help;
  Range range;
};

/** WebAssembly runtime metadata declared by page front matter. */
struct Runtime {
  std::string module;
  std::string application;
  std::string canvas;
};

/** Normalized YAML front matter and project defaults for a page. */
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

/** A typed event connection from a component to an exported function. */
struct Binding {
  std::string event;
  std::string function;
  std::vector<std::string> arguments;
  std::string output;
};

using Properties = std::map<std::string, std::string>;

enum class NodeKind : std::uint8_t { Markdown, Component };

/** A Markdown fragment or Slam UI component in document order. */
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

/** Executable C++/WebAssembly source embedded in a document. */
struct Behavior {
  std::string language;
  std::string source;
  Range range;
  std::vector<std::string> exports;
};

/** Parsed page, including metadata, nodes, behavior, and diagnostics. */
struct Document {
  std::filesystem::path path;
  Metadata metadata;
  std::vector<Node> nodes;
  Behavior behavior;
  std::vector<Diagnostic> diagnostics;
};

/** HTML and any diagnostics produced while rendering a document. */
struct RenderResult {
  std::string html;
  std::vector<Diagnostic> diagnostics;
};

/** Optional renderer hooks and generated-runtime URL settings. */
struct RenderOptions {
  bool unsafeHtml = false;
  std::function<std::string(const std::string &)> resolveUrl;
  std::string runtimeUrl = "/slamhaus-runtime.js";
  std::string moduleUrl;
};

/** One syntax token suitable for semantic highlighting in an editor. */
struct SemanticToken {
  Range range;
  std::string type;
};

/** Parse Markdown and Slamhaus fences into a deterministic document model. */
auto Parse(const std::string &source, const std::filesystem::path &path = {},
           Metadata defaults = {}) -> Document;
/** Render a parsed document to semantic HTML. */
auto Render(const Document &document, const RenderOptions &options = {})
    -> RenderResult;
/** Format source while preserving its meaning and source dialect. */
auto Format(const std::string &source) -> std::string;
/** Tokenize source for editor semantic highlighting. */
auto SemanticTokens(const std::string &source, const std::filesystem::path &path = {})
    -> std::vector<SemanticToken>;
/** Return whether a diagnostic collection contains an error. */
auto HasErrors(const std::vector<Diagnostic> &diagnostics) -> bool;
/** Serialize diagnostics using the stable machine-readable schema. */
auto DiagnosticsJson(const std::vector<Diagnostic> &diagnostics) -> nlohmann::json;
/** Return the language schema used by validation and editor tooling. */
auto SchemaJson() -> nlohmann::json;
/** Escape text for safe insertion into HTML. */
auto EscapeHtml(const std::string &value) -> std::string;
/** Normalize an authored link without changing its route semantics. */
auto AdjustLink(const std::string &value) -> std::string;

} // namespace Language
