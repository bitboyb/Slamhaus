#pragma once

#ifndef HTML_HPP
#define HTML_HPP

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
} // namespace HTML

#endif // HTML_HPP
