#pragma once

#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include <sstream>

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
    
    std::string ParseMarkdown(const std::string& markdown);
    std::string StripParagraphTags(const std::string &html);
    std::string ExtractSiteTitle(const std::string &markdown);
}

#endif // PARSER_HPP
