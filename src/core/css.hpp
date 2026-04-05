#pragma once

#ifndef CSS_HPP
#define CSS_HPP

#include <filesystem>
#include <string>
#include <vector>

namespace CSS {
/// Collects all theme stylesheet paths relative to the theme directory.
/// Paths are returned in sorted order for stable output.
///
/// @param cssPath Path to a directory containing CSS files
/// @return Relative stylesheet paths such as `body.css`
auto CollectStylesheets(const std::filesystem::path &cssPath)
    -> std::vector<std::string>;

/// Builds `<link rel="stylesheet">` tags for the given theme files.
///
/// @param stylesheets Relative stylesheet paths collected from the theme folder
/// @param rootPrefix Relative prefix from the current page to the site root
/// @return HTML stylesheet link tags ready for the page template
auto BuildStylesheetTags(const std::vector<std::string> &stylesheets,
                         const std::string &rootPrefix) -> std::string;
} // namespace CSS

#endif // CSS_HPP
