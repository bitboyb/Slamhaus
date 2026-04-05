#pragma once

#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include "config.hpp"
#include <filesystem>
#include <string>

namespace Generator {
/// Represents the rendered output of a single page build.
struct PageOutput {
  std::string html;
};

/// Renders a single `.slam` document into final HTML using the shared parser
/// and template pipeline.
///
/// @param markdown Raw page source
/// @param templateString Shared site template with placeholders
/// @param stylesheetTags Page-relative stylesheet tags
/// @param faviconHref Page-relative or absolute favicon path
/// @param ini Site-wide config defaults
/// @return The fully rendered page HTML
auto RenderPage(const std::string &markdown, const std::string &templateString,
                const std::string &stylesheetTags,
                const std::string &faviconHref,
                const Config::ConfigINI &ini) -> PageOutput;

/// Generates the full static site from the given content directory.
/// - Parses all `.slam` files into HTML with template and metadata injection
/// - Compiles project `scripts/*.cpp` into WebAssembly using Emscripten
/// - Copies assets and builds sitemap, robots.txt, and 404 page
///
/// @param contentDir Path to your Markdown source directory (e.g. ./content)
/// @param outputDir  Destination for the compiled static site (e.g. ./output)
auto GenerateSite(const std::filesystem::path &contentDir,
                  const std::filesystem::path &outputDir,
                  bool skipConfirmation = false, bool quiet = false) -> bool;

/// Generates a standard Slamhaus project rooted at `projectRoot`.
///
/// @param projectRoot Root directory containing `content/` and `output/`
/// @param skipConfirmation Whether existing output should be deleted without a
/// prompt
/// @param quiet Whether to suppress informational output
/// @return True if the project build succeeded
auto GenerateProject(const std::filesystem::path &projectRoot,
                     bool skipConfirmation = false, bool quiet = false)
    -> bool;
} // namespace Generator

#endif // GENERATOR_HPP
