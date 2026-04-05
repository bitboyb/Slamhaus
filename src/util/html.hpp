#pragma once

#ifndef HTML_HPP
#define HTML_HPP

#include <filesystem>
#include <string>

namespace HTML {
/// Rewrites Markdown-style links S(.smd, .slam) in generated HTML to
/// .html. For example: href="about.slam" -> href="about.html"
///
/// @param html A string of generated HTML content.
/// @return A new HTML string with all `.smd`/`.slam` links converted to
/// `.html`.
auto AdjustLinks(const std::string &html) -> std::string;

/// Converts internal Markdown-like paths (.smd, .slam) to `.html` for use
/// in attributes. For example: /about.slam → /about.html
///
/// @param path A single file path or URL string.
/// @return The path with `.smd`/`.slam` replaced with `.html`, if
/// applicable.
auto AdjustPath(const std::string &path) -> std::string;

/// Returns the relative prefix needed to reference the site root from a page.
/// For example: `blog/post.html` becomes `"../"`.
///
/// @param path The page path relative to the output root
/// @return A relative prefix such as `"../"` or `""`
auto GetRelativeRoot(const std::filesystem::path &path) -> std::string;

/// Prefixes a relative path with the supplied root prefix while leaving
/// absolute, fragment, and external URLs unchanged.
///
/// @param path The asset path to rewrite
/// @param prefix The relative root prefix for the current page
/// @return The page-relative asset path
auto PrefixPath(const std::string &path, const std::string &prefix)
    -> std::string;
} // namespace HTML

#endif // HTML_HPP
