#pragma once

#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>

namespace Parser 
{
    struct ParseState 
    {
        bool inUl = false;
        bool inOl = false;
        bool inCodeBlock = false;
        bool inColumn = false;
    };
    
    std::string ParseMarkdown(const std::string& markdown);
    std::string StripParagraphTags(const std::string &html);
    std::string ExtractSiteTitle(const std::string &markdown);
}

#endif // PARSER_HPP
