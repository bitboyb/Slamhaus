#include "parser.hpp"
#include <regex>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cctype>
#include <vector>

namespace Parser 
{

    std::string ReadTemplateFile(const std::string &templatePath)
    {
        std::ifstream tmplFile(templatePath);
        if (!tmplFile)
        {
            std::cerr << "Error opening template file: " << templatePath << std::endl;
            return "";
        }
        std::ostringstream tmplStream;
        tmplStream << tmplFile.rdbuf();
        return tmplStream.str();
    }

    void ReplacePlaceholder(std::string &tmplStr, const std::string &placeholder, const std::string &value)
    {
        size_t pos = tmplStr.find(placeholder);
        if (pos != std::string::npos)
        {
            tmplStr.replace(pos, placeholder.length(), value);
        }
        else if (placeholder == "{{ content }}")
        {
            std::cerr << "Warning: Content placeholder not found in template. Appending content." << std::endl;
            tmplStr += value;
        }
        else
        {
            std::cerr << "Warning: " << placeholder << " placeholder not found in template." << std::endl;
        }
    }

    std::string ApplyTemplate(const std::string &content,
                              const std::string &templatePath,
                              const std::string &cssContent)
    {
        std::string tmplStr = ReadTemplateFile(templatePath);
        if (tmplStr.empty())
        {
            return content;
        }
        ReplacePlaceholder(tmplStr, "{{ css }}", cssContent);
        ReplacePlaceholder(tmplStr, "{{ content }}", content);
        return tmplStr;
    }

    std::string ReplaceInlineCode(const std::string &text)
    {
        return std::regex_replace(text, std::regex(R"(`([^`]+)`)") , "<code>$1</code>");
    }

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

    static std::string ProcessImages(const std::string &input)
    {
        std::string out = input;
        size_t pos = 0;
        while ((pos = out.find("![", pos)) != std::string::npos)
        {
            size_t endAlt = out.find("]", pos + 2);
            if ((endAlt != std::string::npos) && ((endAlt + 1) < out.size()) && (out[endAlt + 1] == '('))
            {
                size_t endUrl = out.find(")", endAlt + 2);
                if (endUrl != std::string::npos)
                {
                    std::string rawAlt = out.substr(pos + 2, endAlt - (pos + 2));
                    std::string url = out.substr(endAlt + 2, endUrl - (endAlt + 2));
                    std::istringstream iss(rawAlt);
                    std::string token, description, styleAttr;

                    while (iss >> token)
                    {
                        size_t colonPos = token.find(':');
                        if (colonPos != std::string::npos)
                        {
                            std::string key = token.substr(0, colonPos);
                            std::string value = token.substr(colonPos + 1);
                            if ((key == "height") || (key == "width"))
                            {
                                styleAttr += key + ":" + (value == "max" ? "100%" : value) + "; ";
                                continue;
                            }
                        }
                        if (!description.empty())
                        {
                            description += " ";
                        }
                        description += token;
                    }
                    if (description.empty())
                    {
                        description = rawAlt;
                    }
                    std::string imgTag = "<img src=\"" + url + "\" alt=\"" + description + "\"";
                    if (!styleAttr.empty())
                    {
                        imgTag += " style=\"" + styleAttr + "\"";
                    }
                    imgTag += ">";
                    out.replace(pos, endUrl - pos + 1, imgTag);
                    pos += imgTag.size();
                }
                else
                {
                    pos = endAlt + 1;
                }
            }
            else
            {
                pos += 2;
            }
        }
        return out;
    }

    static std::string ProcessLinks(const std::string &input)
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

    std::string ProcessInline(const std::string &text)
    {
        std::string out = text;
        out = ProcessImages(out);
        out = ProcessLinks(out);
        out = ReplaceInlineCode(out);
        out = ReplaceBoldItalic(out);
        out = ReplaceBold(out);
        out = ReplaceItalic(out);
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

    void AppendHeading(std::ostringstream &html, int level, const std::string &line)
    {
        html << "<h" << level << ">" << ProcessInline(line) << "</h" << level << ">\n";
    }

    void AppendParagraph(std::ostringstream &html, const std::string &line)
    {
        html << "<p>" << ProcessInline(line) << "</p>\n";
    }

    void AppendListItem(std::ostringstream &html, const std::string &line)
    {
        html << "<li>" << ProcessInline(line) << "</li>\n";
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

    std::string ParseCodeBlock(std::istream &iss, const std::string &openingLine)
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
        return "<pre><code>" + HighlightCode(codeBlock.str()) + "</code></pre>\n";
    }

    std::string ParseMarkdown(const std::string &markdown)
    {
        std::istringstream iss(markdown);
        std::string line;
        std::ostringstream html;
        bool inUl = false;
        bool inOl = false;
        bool inCodeBlock = false;
        while (std::getline(iss, line))
        {
            if (line.rfind("```", 0) == 0)
            {
                if (!inCodeBlock)
                {
                    CloseLists(html, inUl, inOl);
                    inCodeBlock = true;
                    html << ParseCodeBlock(iss, line);
                    inCodeBlock = false;
                }
                continue;
            }
            if (line.empty())
            {
                CloseLists(html, inUl, inOl);
                continue;
            }
            if (line.rfind("!iframe[", 0) == 0 && line.back() == ']')
            {
                CloseLists(html, inUl, inOl);
                size_t start = line.find('[');
                size_t end = line.find(']');
                if (start != std::string::npos && end != std::string::npos && end > start)
                {
                    std::string url = line.substr(start + 1, end - start - 1);
                    html << "<div class=\"embed-container\">\n"
                         << "<iframe src=\"" << url << "\" "
                         << "frameborder=\"0\" allowfullscreen "
                         << "referrerpolicy=\"strict-origin-when-cross-origin\"></iframe>\n"
                         << "</div>\n";
                }
                continue;
            }      
            if ((std::regex_match(line, std::regex(R"(^-{3,}\s*$)"))) || (line == "---"))
            {
                CloseLists(html, inUl, inOl);
                html << "<hr>\n";
                continue;
            }
            if ((!line.empty()) && (line[0] == '|'))
            {
                CloseLists(html, inUl, inOl);
                html << ParseTable(iss, line);
                continue;
            }
            if (line.find("# ") == 0)
            {
                CloseLists(html, inUl, inOl);
                AppendHeading(html, 1, line.substr(2));
            }
            else if (line.find("## ") == 0)
            {
                CloseLists(html, inUl, inOl);
                AppendHeading(html, 2, line.substr(3));
            }
            else if (line.find("### ") == 0)
            {
                CloseLists(html, inUl, inOl);
                AppendHeading(html, 3, line.substr(4));
            }
            else if (line.find("- ") == 0)
            {
                if (!inUl)
                {
                    CloseLists(html, inUl, inOl);
                    html << "<ul>\n";
                    inUl = true;
                }
                AppendListItem(html, line.substr(2));
            }
            else if (IsNumberedListLine(line))
            {
                if (!inOl)
                {
                    CloseLists(html, inUl, inOl);
                    html << "<ol>\n";
                    inOl = true;
                }
                size_t pos = line.find(". ");
                if (pos != std::string::npos)
                {
                    AppendListItem(html, line.substr(pos + 2));
                }
                else
                {
                    AppendListItem(html, line);
                }
            }
            else
            {
                CloseLists(html, inUl, inOl);
                AppendParagraph(html, line);
            }
        }
        CloseLists(html, inUl, inOl);
        return html.str();
    }

    std::string StripParagraphTags(const std::string &html)
    {
        std::string result = html;
        if ((result.size() >= 7) && (result.substr(0, 3) == "<p>") && (result.substr(result.size() - 4) == "</p>"))
        {
            result = result.substr(3, result.size() - 7);
        }
        return result;
    }
}