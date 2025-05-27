#pragma once

#ifndef SNIPPET_HPP
#define SNIPPET_HPP

#include <string>
#include <sstream>
#include "parser.hpp"

namespace Snippet
{
    /// Replaces inline backtick code with <code> elements.
    /// Converts `example` into <code>example</code> in a Markdown line.
    ///
    /// @param text Input Markdown line with potential backticks
    /// @return HTML string with inline code replaced
    std::string ReplaceInlineCode(const std::string &text);

    /// Detects and parses either a code block (``` syntax) or table row.
    /// Outputs HTML into the stream and updates parse state.
    ///
    /// - Code blocks are wrapped in <pre><code>...</code></pre>
    /// - Markdown tables are parsed row by row until the end
    ///
    /// @param line  The current line to evaluate
    /// @param iss   Input stream for multi-line code or table parsing
    /// @param html  Output HTML stream
    /// @param pState Current parser state (used for closing/opening tags)
    /// @return True if the line was handled as a code block or table
    bool HandleCodeOrTable(const std::string& line, 
                           std::istringstream& iss, 
                           std::ostringstream& html, 
                           Parser::ParseState& pState);
}

#endif // SNIPPET_HPP
