#pragma once

#ifndef SECTION_HPP
#define SECTION_HPP

#include "parser.hpp"
#include <string>

namespace Section {
/// Handles Slamhaus block elements like :section, :hero, and :column.
/// Detects open/close lines and outputs corresponding HTML.
///
/// - Buffers column content until :/column is encountered.
/// - Applies section and hero attributes like id and alignment.
/// - Works with Parser::ParseState and Parser::ColumnState.
///
/// @param line     The raw line from the Markdown file
/// @param html     The output HTML stream
/// @param pState   The current parsing state
/// @param cState   The column block state (for :column)
/// @param seo      Optional SEO data (for parsing metadata within columns)
/// @return True if the line was handled by this module
auto HandleBlockElements(const std::string &line, std::ostringstream &html,
                         Parser::ParseState &pState,
                         Parser::ColumnState &cState, SEO::SEOData *seo)
    -> bool;
} // namespace Section

#endif // SECTION_HPP
