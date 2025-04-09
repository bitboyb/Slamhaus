#ifndef TEXT_HPP
#define TEXT_HPP

#include "parser.hpp"
#include <string>

namespace Text
{
    std::string ReplaceBoldItalic(const std::string &text);
    std::string ReplaceBold(const std::string &text);
    std::string ReplaceItalic(const std::string &text);
    std::string ProcessLinks(const std::string &input);

    bool IsNumberedListLine(const std::string &line);
    bool IsUnorderedListLine(const std::string &line);
    bool IsHorizontalRuleLine(const std::string &line);
    bool IsTableLine(const std::string &line);
    bool IsHeading1(const std::string &line);
    bool IsHeading2(const std::string &line);
    bool IsHeading3(const std::string &line);

    void AppendHeading(std::ostringstream &html, 
                       int level, 
                       const std::string &line);
    void AppendParagraph(std::ostringstream &html, 
                         const std::string &line);
    void AppendListItem(std::ostringstream &html, 
                        const std::string &line);
    void CloseLists(std::ostringstream &html, 
                    Parser::ParseState& pState);
    std::string ParseTable(std::istream &iss, 
                           const std::string &firstLine);
}

#endif // TEXT_HPP
