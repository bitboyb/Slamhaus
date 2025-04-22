#pragma once

#ifndef EMBED_HPP
#define EMBED_HPP

#include <string>
#include "parser.hpp"

namespace Embed
{
    bool HandleEmbeds(const std::string& line, 
                      std::ostringstream& html, 
                      Parser::ParseState& pState);
}

#endif // EMBED_HPP
