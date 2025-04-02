#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>

namespace Parser 
{
    std::string ParseMarkdown(const std::string& markdown);
    
    std::string ApplyTemplate(const std::string &content, 
                              const std::string &templatePath, 
                              const std::string &cssContent);
}

#endif // PARSER_HPP
