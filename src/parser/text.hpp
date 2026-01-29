#pragma once

#ifndef TEXT_HPP
#define TEXT_HPP

#include "parser.hpp"
#include <sstream>
#include <string>

namespace Text {
/// Closes unclosed paragraph tags at the end of document parsing.
auto CloseParagraphs(std::ostringstream &html) -> void;

/// Closes any open <ul> or <ol> lists in the HTML stream.
///
/// @param html Output stream to write HTML to
/// @param pState Current parser state
auto CloseLists(std::ostringstream &html, Parser::ParseState &pState) -> void;

/// Checks whether a line represents a Markdown table row.
///
/// @param line Line of markdown
/// @return True if line starts with a pipe `|`
auto IsTableLine(const std::string &line) -> bool;

/// Parses a Markdown-style table from a stream and outputs HTML.
///
/// @param iss       Stream containing lines after the table start
/// @param firstLine The initial line that triggered the table
/// @return HTML string containing <table> markup
auto ParseTable(std::istream &iss, const std::string &firstLine) -> std::string;

/// Parses all standard text blocks (headings, paragraphs, lists, rules).
///
/// - Handles paragraphs, headers (#, ##, etc), lists, horizontal rules
/// - Applies inline formatting such as links and bold/italic
///
/// @param line   Current line to process
/// @param html   Output stream containing generated HTML
/// @param pState Current parser state
/// @return True if the line was handled as a text block
auto HandleTextBlocks(const std::string &line, std::ostringstream &html,
                      Parser::ParseState &pState) -> bool;
} // namespace Text

#endif // TEXT_HPP
