#pragma once

#ifndef SNIPPET_HPP
#define SNIPPET_HPP

#include <string>
#include "parser.hpp"

namespace Snippet
{
    std::string ReplaceInlineCode(const std::string &text);

    bool HandleCodeOrTable(const std::string& line, 
                           std::istringstream& iss, 
                           std::ostringstream& html, 
                           Parser::ParseState& pState);
}

#endif // SNIPPET_HPP
