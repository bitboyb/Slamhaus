#pragma once

#ifndef TEXT_HPP
#define TEXT_HPP

#include "parser.hpp"
#include <string>

namespace Text
{
    void CloseParagraphs(std::ostringstream& html);
    bool IsTableLine(const std::string &line);

    void CloseLists(std::ostringstream &html, 
                    Parser::ParseState& pState);
                    
    std::string ParseTable(std::istream &iss, 
                           const std::string &firstLine);

    bool HandleTextBlocks(const std::string& line, 
                          std::ostringstream& html, 
                          Parser::ParseState& pState);
}

#endif // TEXT_HPP
