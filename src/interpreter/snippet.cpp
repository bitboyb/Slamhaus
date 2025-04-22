#include <regex>
#include <sstream>
#include <iostream>
#include <cctype>
#include "text.hpp"

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

    bool HandleCodeOrTable(const std::string& line, 
                           std::istringstream& iss, 
                           std::ostringstream& html, 
                           Parser::ParseState& pState)
    {
        if (Snippet::IsCodeBlockLine(line)) 
        {
            if (!pState.inCodeBlock) 
            {
                Text::CloseLists(html, pState);
                pState.inCodeBlock = true;
                html << Snippet::ParseCodeBlock(iss, line);
                pState.inCodeBlock = false;
            }
            return true;
        }

        if (Text::IsTableLine(line)) 
        {
            Text::CloseLists(html, pState);
            html << Text::ParseTable(iss, line);
            return true;
        }

        return false;
    }
}