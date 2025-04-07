#ifndef SNIPPET_HPP
#define SNIPPET_HPP

#include <string>

namespace Snippet
{
    std::string ReplaceInlineCode(const std::string &text);

    std::string HighlightCode(const std::string& code);

    std::string ParseCodeBlock(std::istream &iss, 
                               const std::string &openingLine);
}

#endif // SNIPPET_HPP
