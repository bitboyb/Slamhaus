#pragma once

#ifndef NAV_HPP
#define NAV_HPP

#include <string>

namespace Nav
{
    /// Parses a Markdown-formatted navigation list into nested HTML <ul><li> structure.
    /// Converts `.md` links to `.html`, handles indentation for nested lists.
    ///
    /// Example Markdown:
    ///   - [Home](index.md)
    ///       - [About](about.md)
    ///
    /// @param navMarkdown Raw Markdown string for navigation.
    /// @return HTML <nav> block with responsive toggle support and nested lists.
    std::string ParseNavigation(const std::string& navMarkdown);
}

#endif // NAV_HPP
