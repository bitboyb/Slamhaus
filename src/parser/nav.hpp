#pragma once

#ifndef NAV_HPP
#define NAV_HPP

#include <string>

namespace Nav {
/// Parses a Markdown-formatted navigation list into nested HTML `<ul><li>`
/// structure. Converts `.slam` links to `.html`, handles indentation for nested
/// lists.
///
/// Example Markdown:
///   - [Home](index.slam)
///       - [About](about.slam)
///
/// @param navMarkdown Raw Markdown string for navigation.
/// @return HTML `<nav>` block with responsive toggle support and nested lists.
auto ParseNavigation(const std::string &navMarkdown) -> std::string;
} // namespace Nav

#endif // NAV_HPP
