#include "parser.hpp"
#include <regex>
#include <sstream>
#include <iostream>
#include <cctype>
#include <vector>

namespace Text
{
    std::string ReplaceBoldItalic(const std::string &text)
    {
        return std::regex_replace(text, std::regex(R"(\*\*\*(.+?)\*\*\*)"), "<strong><em>$1</em></strong>");
    }

    std::string ReplaceBold(const std::string &text)
    {
        return std::regex_replace(text, std::regex(R"(\*\*(.+?)\*\*)"), "<strong>$1</strong>");
    }

    std::string ReplaceItalic(const std::string &text)
    {
        return std::regex_replace(text, std::regex(R"(\*(.+?)\*)"), "<em>$1</em>");
    }

    std::string ProcessLinks(const std::string &input)
    {
        std::string out = input;
        size_t pos = 0;
        while ((pos = out.find("[", pos)) != std::string::npos)
        {
            size_t endText = out.find("]", pos + 1);
            if ((endText != std::string::npos) && ((endText + 1) < out.size()) && (out[endText + 1] == '('))
            {
                size_t endUrl = out.find(")", endText + 2);
                if (endUrl != std::string::npos)
                {
                    std::string linkText = out.substr(pos + 1, endText - pos - 1);
                    std::string url = out.substr(endText + 2, endUrl - (endText + 2));
                    std::string linkTag = "<a href=\"" + url + "\">" + linkText + "</a>";
                    out.replace(pos, endUrl - pos + 1, linkTag);
                    pos += linkTag.size();
                }
                else
                {
                    pos = endText + 1;
                }
            }
            else
            {
                pos++;
            }
        }

        return out;
    }

    bool IsNumberedListLine(const std::string &line)
    {
        size_t pos = 0;
        while ((pos < line.size()) && std::isdigit(line[pos]))
        {
            pos++;
        }
        return ((pos > 0) && (pos + 1 < line.size()) && (line[pos] == '.') && (line[pos + 1] == ' '));
    }

    bool IsUnorderedListLine(const std::string &line) 
    {
        return line.find("- ") == 0;
    }

    bool IsHorizontalRuleLine(const std::string &line) 
    {
        return std::regex_match(line, std::regex(R"(^-{3,}\s*$)")) || (line == "---");
    }

    bool IsTableLine(const std::string &line) 
    {
        return !line.empty() && line[0] == '|';
    }

    bool IsHeading1(const std::string &line) 
    {
        return line.find("# ") == 0;
    }

    bool IsHeading2(const std::string &line) 
    {
        return line.find("## ") == 0;
    }

    bool IsHeading3(const std::string &line) 
    {
        return line.find("### ") == 0;
    }

    void AppendHeading(std::ostringstream &html, int level, const std::string &line)
    {
        html << "<h" << level << ">" << Parser::ProcessInline(line) << "</h" << level << ">\n";
    }

    void AppendParagraph(std::ostringstream &html, const std::string &line)
    {
        html << "<p>" << Parser::ProcessInline(line) << "</p>\n";
    }

    void AppendListItem(std::ostringstream &html, const std::string &line)
    {
        html << "<li>" << Parser::ProcessInline(line) << "</li>\n";
    }

    void CloseLists(std::ostringstream &html, bool &inUl, bool &inOl)
    {
        if (inUl)
        {
            html << "</ul>\n";
            inUl = false;
        }
        if (inOl)
        {
            html << "</ol>\n";
            inOl = false;
        }
    }

    std::string ParseTable(std::istream &iss, const std::string &firstLine)
    {
        std::vector<std::string> tableLines;
        tableLines.push_back(firstLine);
        std::string next;
        while (std::getline(iss, next))
        {
            if ((!next.empty()) && (next[0] == '|'))
            {
                tableLines.push_back(next);
            }
            else
            {
                break;
            }
        }
        if (tableLines.empty())
        {
            std::cerr << "Error: Table has no content." << std::endl;
            return "";
        }
        std::ostringstream tableHtml;
        tableHtml << "<table>\n";
        bool headerFound = false;

        if (tableLines.size() >= 2)
        {
            std::regex sepRegex(R"(^\|[\-\:\s\|]+$)");

            if (std::regex_match(tableLines[1], sepRegex))
            {
                headerFound = true;
            }
        }
        for (size_t i = 0; i < tableLines.size(); i++)
        {
            if (headerFound && (i == 1))
            {
                continue;
            }
            tableHtml << "<tr>";
            std::istringstream rowStream(tableLines[i]);
            std::string cell;
            if ((!tableLines[i].empty()) && (tableLines[i][0] == '|'))
            {
                std::getline(rowStream, cell, '|');
            }
            while (std::getline(rowStream, cell, '|'))
            {
                cell = std::regex_replace(cell, std::regex(R"(^\s+|\s+$)"), "");
                if (headerFound && (i == 0))
                {
                    tableHtml << "<th>" << cell << "</th>";
                }
                else
                {
                    tableHtml << "<td>" << cell << "</td>";
                }
            }
            tableHtml << "</tr>\n";
        }
        tableHtml << "</table>\n";
        return tableHtml.str();
    }
}