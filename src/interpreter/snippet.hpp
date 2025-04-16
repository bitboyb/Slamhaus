#pragma once

#ifndef SNIPPET_HPP
#define SNIPPET_HPP

#include <string>

namespace Snippet
{
    bool IsCodeBlockLine(const std::string &line);

    std::string ReplaceInlineCode(const std::string &text);
    std::string ParseCodeBlock(std::istream &iss, 
                               const std::string &openingLine);
}

#endif // SNIPPET_HPP
