#pragma once

#ifndef SCRIPT_HPP
#define SCRIPT_HPP

#include <string>
#include <vector>
#include "../parser/parser.hpp"

namespace Script
{
    bool HandleScript(const std::string& line,
                      std::istringstream& iss,
                      std::ostringstream& html,
                      Parser::ParseState& pState);
}

#endif // SCRIPT_HPP