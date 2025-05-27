#pragma once

#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include <sstream>
#include "seo.hpp"

namespace Parser
{
    /// Represents the current state of parsing.
    /// Tracks whether we're inside lists, code blocks, forms, or column blocks.
    struct ParseState
    {
        bool inUl = false;
        bool inOl = false;
        bool inCodeBlock = false;
        bool inColumn = false;
        bool inForm = false;
        bool isBuffering = false;
    };

    /// Tracks state for managing multi-column layouts during parsing.
    struct ColumnState
    {
        std::ostringstream columnBuffer;
        std::vector<std::string> columnParts;
        std::vector<std::string> columnOpenLines;
    };

    /// Parses Slamhaus-flavored Markdown into HTML.
    /// Includes support for metadata, custom blocks, forms, embeds, and scripts.
    ///
    /// @param markdown The raw Markdown string
    /// @param seo Optional pointer to SEOData to populate with parsed metadata
    /// @return HTML string generated from parsed Markdown
    std::string ParseMarkdown(const std::string& markdown,
                              SEO::SEOData* seo = nullptr);
}

#endif // PARSER_HPP
