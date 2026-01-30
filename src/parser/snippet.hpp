#pragma once

#ifndef SNIPPET_HPP
#define SNIPPET_HPP

#include "parser.hpp"
#include <sstream>
#include <string>

namespace Snippet {
/// Replaces inline backtick code with &lt;code&gt; elements.
/// Converts `example` into `&lt;code&gt;example&lt;/code&gt;` in a Markdown
/// line.
///
/// @param text Input Markdown line with potential backticks
/// @return HTML string with inline code replaced
auto ReplaceInlineCode(const std::string &text) -> std::string;

/// Detects and parses either a code block (``` syntax) or table row.
/// Outputs HTML into the stream and updates parse state.
///
/// - Code blocks are wrapped in
/// &lt;pre&gt;&lt;code&gt;...&lt;/code&gt;&lt;/pre&gt;
/// - Markdown tables are parsed row by row until the end
///
/// @param line  The current line to evaluate
/// @param iss   Input stream for multi-line code or table parsing
/// @param html  Output HTML stream
/// @param pState Current parser state (used for closing/opening tags)
/// @return True if the line was handled as a code block or table
auto HandleCodeOrTable(const std::string &line, std::istringstream &iss,
                       std::ostringstream &html, Parser::ParseState &pState)
    -> bool;
} // namespace Snippet

#endif // SNIPPET_HPP
