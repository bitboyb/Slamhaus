#pragma once

#ifndef META_HPP
#define META_HPP

#include <string>
#include <vector>

namespace Meta {
/// Generates standard meta files for a static site build:
/// - robots.txt for search engine crawling rules
/// - sitemap.xml listing all `.html` pages
/// - 404.html fallback page with linked site stylesheets
///
/// @param siteURL    Base URL for sitemap generation
/// @param stylesheetTags Stylesheet tags to include in 404 page
/// @param outputDir  Directory to write the files to (e.g. ./build)
/// @param pages      List of generated HTML page filenames
auto GenerateSiteMetaFiles(const std::string &siteURL,
                           const std::string &stylesheetTags,
                           const std::string &outputDir,
                           const std::vector<std::string> &pages) -> void;
} // namespace Meta

#endif // META_HPP
