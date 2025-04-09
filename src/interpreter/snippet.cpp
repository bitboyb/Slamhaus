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
    
    std::string HighlightCode(const std::string& code)
    {
        std::string result = code;
        result = std::regex_replace(
            result,
            std::regex(R"(\b(if|else|for|while|return|break|continue|switch|case|default|class|struct|const|void|int|float|double|bool|char|unsigned|signed)\b)"),
            "<span class=\"keyword\">$1</span>"
        );
        result = std::regex_replace(
            result,
            std::regex(R"_STR("([^"]*)")_STR"),
            "\"<span class=\"string\">$1</span>\""
        );
        result = std::regex_replace(
            result,
            std::regex(R"(\b\d+(\.\d+)?\b)"),
            "<span class=\"number\">$&</span>"
        );
        result = std::regex_replace(
            result,
            std::regex(R"(//[^\n]*)"),
            "<span class=\"comment\">$&</span>"
        );
        result = std::regex_replace(
            result,
            std::regex(R"(\b([A-Za-z_]\w*)\s*(?=\())"),
            "<span class=\"function\">$1</span>"
        );
        return result;
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