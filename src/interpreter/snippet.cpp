#include <regex>
#include <sstream>
#include <iostream>
#include <cctype>

namespace Snippet
{
    bool IsCodeBlockLine(const std::string &line) 
    {
        return line.rfind("```", 0) == 0;
    }
    
    std::string ReplaceInlineCode(const std::string &text)
    {
        return std::regex_replace(text, std::regex(R"(`([^`]+)`)") , "<code>$1</code>");
    }

    std::string ParseCodeBlock(std::istream &iss, 
                               const std::string &openingLine)
    {
        std::ostringstream codeBlock;
        std::string line;
        while (std::getline(iss, line))
        {
            if (line.rfind("```", 0) == 0)
            {
                break;
            }
            codeBlock << line << "\n";
        }
        return "<pre><code>" + codeBlock.str() + "</code></pre>\n";
    }
}