#include "parser.hpp"
#include <regex>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cctype>
#include <vector>

std::string ParseTable(std::istream &iss, const std::string &firstLine)
{
    std::vector<std::string> tableLines;
    tableLines.push_back(firstLine);
    std::string next;
    while (std::getline(iss, next))
    {
        if (!next.empty() && next[0] == '|')
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
        if (headerFound && i == 1)
        {
            continue;
        }
        tableHtml << "<tr>";
        std::istringstream rowStream(tableLines[i]);
        std::string cell;
        if (!tableLines[i].empty() && tableLines[i][0] == '|')
        {
            std::getline(rowStream, cell, '|');
        }

        while (std::getline(rowStream, cell, '|'))
        {
            cell = std::regex_replace(cell, std::regex("^\\s+|\\s+$"), "");
            if (headerFound && i == 0)
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

// Helper to remove wrapping <p> tags if present.
std::string StripParagraphTags(const std::string &html)
{
    std::string result = html;
    if(result.size() >= 7 && result.substr(0, 3) == "<p>" && result.substr(result.size()-4) == "</p>")
    {
         result = result.substr(3, result.size()-7);
    }
    return result;
}

std::string LoadCSS(const std::string &cssPath) 
{
    std::ifstream cssFile(cssPath);
    if (!cssFile) 
    {
        std::cerr << "Error opening CSS file: " << cssPath << std::endl;
        return "";
    }
    std::ostringstream cssStream;
    cssStream << cssFile.rdbuf();
    return cssStream.str();
}

// Applies the template from a file, replacing the placeholders.
std::string ApplyTemplate(const std::string &content, 
                          const std::string &templatePath, 
                          const std::string &cssContent)
{
    std::ifstream tmplFile(templatePath);
    if (!tmplFile) 
    {
        std::cerr << "Error opening template file: " << templatePath << std::endl;
        return content;
    }
    std::ostringstream tmplStream;
    tmplStream << tmplFile.rdbuf();
    std::string tmplStr = tmplStream.str();

    std::string cssPlaceholder = "{{ css }}";
    size_t pos = tmplStr.find(cssPlaceholder);
    if (pos != std::string::npos) 
    {
        tmplStr.replace(pos, cssPlaceholder.length(), cssContent);
    }

    std::string contentPlaceholder = "{{ content }}";
    pos = tmplStr.find(contentPlaceholder);
    if (pos != std::string::npos) 
    {
        tmplStr.replace(pos, contentPlaceholder.length(), content);
    } 
    else 
    {
        tmplStr += content;
    }
    return tmplStr;
}

// Processes inline elements: images, links, inline code, bold, and italics.
std::string ProcessInline(const std::string &text)
{
    std::string out = text;
    size_t pos = 0;
    
    // Process images with optional size attributes in alt text.
    while ((pos = out.find("![", pos)) != std::string::npos)
    {
        size_t endAlt = out.find("]", pos + 2);
        if (endAlt != std::string::npos && endAlt + 1 < out.size() && out[endAlt + 1] == '(')
        {
            size_t endUrl = out.find(")", endAlt + 2);
            if (endUrl != std::string::npos)
            {
                std::string rawAlt = out.substr(pos + 2, endAlt - (pos + 2));
                std::string url = out.substr(endAlt + 2, endUrl - (endAlt + 2));
                
                // Split the rawAlt into tokens to detect size attributes.
                std::istringstream iss(rawAlt);
                std::string token;
                std::string description;
                std::string styleAttr;
                while (iss >> token)
                {
                    size_t colonPos = token.find(':');
                    if (colonPos != std::string::npos)
                    {
                        std::string key = token.substr(0, colonPos);
                        std::string value = token.substr(colonPos + 1);
                        if (key == "height" || key == "width")
                        {
                            // If the value is "max", set it to 100%
                            if (value == "max")
                                styleAttr += key + ":100%; ";
                            else
                                styleAttr += key + ":" + value + "; ";
                            continue; // Skip adding this token to description.
                        }
                    }
                    if (!description.empty())
                        description += " ";
                    description += token;
                }
                if (description.empty())
                    description = rawAlt;
                
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
    
    pos = 0;
    // Process links.
    while ((pos = out.find("[", pos)) != std::string::npos)
    {
        size_t endText = out.find("]", pos + 1);
        if (endText != std::string::npos && endText + 1 < out.size() && out[endText + 1] == '(')
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
    
    // Process inline code: `code`
    {
        std::regex inlineCodeRegex(R"(`([^`]+)`)");
        out = std::regex_replace(out, inlineCodeRegex, "<code>$1</code>");
    }
    
    // Process bold italic: ***text***
    {
        std::regex boldItalicRegex(R"(\*\*\*(.+?)\*\*\*)");
        out = std::regex_replace(out, boldItalicRegex, "<strong><em>$1</em></strong>");
    }
    
    // Process bold: **text**
    {
        std::regex boldRegex(R"(\*\*(.+?)\*\*)");
        out = std::regex_replace(out, boldRegex, "<strong>$1</strong>");
    }
    
    // Process italics: *text*
    {
        std::regex italicRegex(R"(\*(.+?)\*)");
        out = std::regex_replace(out, italicRegex, "<em>$1</em>");
    }
    
    return out;
}
    
bool IsNumberedListLine(const std::string &line)
{
    size_t pos = 0;
    while (pos < line.size() && std::isdigit(line[pos]))
    {
        pos++;
    }
    return (pos > 0 && pos + 1 < line.size() && line[pos] == '.' && line[pos + 1] == ' ');
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

// Parses a code block from the stream, starting at the first line (which is the opening ```).
std::string ParseCodeBlock(std::istream &iss, const std::string &openingLine)
{
    std::ostringstream codeBlock;
    // Optionally, you can capture language specifier from openingLine if needed.
    std::string line;
    while (std::getline(iss, line))
    {
        if (line.rfind("```", 0) == 0)
        {
            // End of code block.
            break;
        }
        // Escape HTML special characters if desired.
        codeBlock << line << "\n";
    }
    return "<pre><code>" + codeBlock.str() + "</code></pre>\n";
}

std::string ParseMarkdown(const std::string& markdown)
{
    std::istringstream iss(markdown);
    std::string line;
    std::ostringstream html;
    bool inUl = false;
    bool inOl = false;
    bool inCodeBlock = false;
    
    while (std::getline(iss, line))
    {
        // If line starts with triple backticks, toggle code block mode.
        if (line.rfind("```", 0) == 0)
        {
            if (!inCodeBlock)
            {
                // Starting a code block: close any open lists.
                CloseLists(html, inUl, inOl);
                inCodeBlock = true;
                // Process the code block and append its HTML.
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
        
        // Section break: line of three or more dashes.
        if (std::regex_match(line, std::regex(R"(^-{3,}\s*$)")) || line == "---")
        {
            CloseLists(html, inUl, inOl);
            html << "<hr>\n";
            continue;
        }
        
        // Table: if line starts with a pipe.
        if (!line.empty() && line[0] == '|')
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
