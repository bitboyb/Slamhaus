#pragma once

#ifndef META_HPP
#define META_HPP

#include <vector>
#include <string>

namespace Meta
{
    /// Generates standard meta files for a static site build:
    /// - robots.txt for search engine crawling rules
    /// - sitemap.xml listing all `.html` pages
    /// - 404.html fallback page with optional embedded CSS
    ///
    /// @param siteURL    Base URL for sitemap generation
    /// @param cssContent Inline CSS string to include in 404 page
    /// @param outputDir  Directory to write the files to (e.g. ./build)
    /// @param pages      List of generated HTML page filenames
    void GenerateSiteMetaFiles(const std::string &siteURL,
                               const std::string &cssContent,
                               const std::string &outputDir,
                               const std::vector<std::string> &pages);
}

#endif // META_HPP
