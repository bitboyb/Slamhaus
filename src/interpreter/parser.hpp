#pragma once

#include "seo.hpp"
#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include <sstream>
#include "../core/config.hpp"

namespace Parser 
{
    struct ParseState 
    {
        bool inUl = false;
        bool inOl = false;
        bool inCodeBlock = false;
        bool inColumn = false;
    };

    struct ColumnState
    {
        std::ostringstream columnBuffer;
        std::vector<std::string> columnParts;
        std::vector<std::string> columnOpenLines;
    };
    
    std::string ParseMarkdown(const std::string &markdown, 
                              SEO::SEOData *seo = nullptr);
}

#endif // PARSER_HPP
