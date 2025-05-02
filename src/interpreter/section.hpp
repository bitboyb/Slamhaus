#pragma once

#ifndef SECTION_HPP
#define SECTION_HPP

#include <string>
#include "parser.hpp"

namespace Section
{
    bool HandleBlockElements(const std::string& line,
                             std::ostringstream& html,
                             Parser::ParseState& pState,
                             Parser::ColumnState& cState,
                             SEO::SEOData* seo);

}

#endif // SECTION_HPP
