#pragma once

#ifndef ELEMENT_HPP
#define ELEMENT_HPP

#include <string>
#include <sstream>
#include "parser.hpp"

namespace Interaction
{
    /// Handles interactive elements like buttons, inputs, and textareas.
    /// Parses lines like `?button[]`, `?input[]`, and `?textarea[]` and emits HTML.
    ///
    /// @param line   The raw Markdown line to evaluate
    /// @param html   The output HTML stream to append to
    /// @param pState The current parser state for list/form handling
    /// @return       True if the line was successfully handled and emitted as HTML
    bool IsInteraction(const std::string& line, 
                       std::ostringstream& html, 
                       Parser::ParseState& pState);
}

#endif // ELEMENT_HPP
