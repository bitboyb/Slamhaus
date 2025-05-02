#pragma once

#ifndef FORM_HPP
#define FORM_HPP

#include <string>
#include <sstream>
#include "parser.hpp"

namespace Form
{
    bool HandleFormElements(const std::string& line,
                             std::ostringstream& html,
                             Parser::ParseState& pState);
}

#endif // FORM_HPP
