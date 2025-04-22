#pragma once

#ifndef ELEMENT_HPP
#define ELEMENT_HPP

#include <string>
#include "parser.hpp"

namespace Element
{
    bool HandleElementLines(const std::string& line, 
                            std::ostringstream& html, 
                            Parser::ParseState& pState);
}

#endif // ELEMENT_HPP
