#pragma once

#ifndef HTML_HPP
#define HTML_HPP

#include <string>

namespace HTML
{
    /// Rewrites Markdown-style links (`.md`, `.smd`, `.slam`) in generated HTML to `.html`.
    /// For example: href="about.md" → href="about.html"
    ///
    /// @param html A string of generated HTML content.
    /// @return A new HTML string with all `.md`/`.smd`/`.slam` links converted to `.html".
    std::string AdjustLinks(const std::string &html);

    /// Converts internal Markdown-like paths (.md, .smd, .slam) to `.html` for use in attributes.
    /// For example: /about.md → /about.html
    ///
    /// @param path A single file path or URL string.
    /// @return The path with `.md`/`.smd`/`.slam` replaced with `.html`, if applicable.
    std::string AdjustPath(const std::string &path);
}

#endif // HTML_HPP
