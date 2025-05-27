#pragma once

#ifndef FORM_HPP
#define FORM_HPP

#include <string>
#include <sstream>
#include "parser.hpp"

namespace Form
{
    /// Handles Slamhaus form block syntax.
    /// Supports opening and closing tags like:
    ///   ?form[action:"/submit":method:"post"](#form-id)
    ///   ?/form[]()
    ///
    /// Emits a valid `<form>` HTML block with parsed attributes.
    /// Handles form state tracking via `ParseState`.
    ///
    /// @param line   Current line from Markdown
    /// @param html   Output HTML buffer
    /// @param pState Parser state (tracks inside-form status)
    /// @return       True if the line was a form tag and was handled
    bool HandleFormElements(const std::string& line,
                            std::ostringstream& html,
                            Parser::ParseState& pState);
}

#endif // FORM_HPP
