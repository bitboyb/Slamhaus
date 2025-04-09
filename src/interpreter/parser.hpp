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
    
    std::string ProcessInline(const std::string &text);
    
    std::string ParseMarkdown(const std::string& markdown);
    
    std::string ApplyTemplate(const std::string &content, 
                              const std::string &templatePath, 
                              const std::string &cssContent);

    std::string StripParagraphTags(const std::string &html);
}

#endif // PARSER_HPP
