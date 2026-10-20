#pragma once

#include "language/language.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace Project {

struct SourceFile {
  std::filesystem::path path;
  std::string content;
};

struct BuildArtifact {
  std::filesystem::path path;
  std::string content;
};

struct Workspace {
  std::vector<SourceFile> files;
};

struct WorkspaceResult {
  bool success = false;
  std::vector<Language::Diagnostic> diagnostics;
  std::vector<BuildArtifact> artifacts;
};

struct Options {
  std::filesystem::path root = ".";
  std::filesystem::path output;
  bool noWasm = false;
  bool strict = false;
  bool dryRun = false;
  bool quiet = false;
};

struct Result {
  bool success = false;
  std::vector<Language::Diagnostic> diagnostics;
  std::vector<std::filesystem::path> outputs;
};

struct InitOptions {
  std::filesystem::path root = ".";
  bool force = false;
};

enum class ExampleProfile : std::uint8_t { Minimal, Complete };

struct ExampleOptions {
  std::filesystem::path root = ".";
  ExampleProfile profile = ExampleProfile::Minimal;
  bool force = false;
};

struct UrlResolution {
  std::string value;
  std::filesystem::path target;
  std::string error;
  bool local = false;
};

auto Init(const InitOptions &options) -> Result;
auto Examples(const ExampleOptions &options) -> Result;
auto Compile(const Workspace &workspace, bool unsafeHtml = false) -> WorkspaceResult;
auto Check(const std::filesystem::path &target, bool strict = false) -> Result;
auto Build(const Options &options) -> Result;
auto Doctor(const std::filesystem::path &root = ".") -> nlohmann::json;
auto BlankProjectFiles() -> std::vector<std::pair<std::filesystem::path, std::string>>;
auto ExampleProjectFiles(ExampleProfile profile)
    -> std::vector<std::pair<std::filesystem::path, std::string>>;
auto ResolveUrl(const std::string &authored, const std::filesystem::path &source,
                const std::filesystem::path &pages, const std::string &basePath = "/",
                bool partial = false) -> UrlResolution;

} // namespace Project
