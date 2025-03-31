#include "parser.hpp"
#include <sstream>
#include <fstream>
#include <iostream>

#include <sstream>
#include <fstream>
#include <iostream>
#include <cctype>

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

std::string ApplyTemplate(const std::string &content, const std::string &templatePath, const std::string &cssContent)
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

std::string ProcessInline(const std::string &text)
{
    std::string out = text;
    size_t pos = 0;
    while ((pos = out.find("![", pos)) != std::string::npos)
    {
        size_t endAlt = out.find("]", pos + 2);
        if (endAlt != std::string::npos && endAlt + 1 < out.size() && out[endAlt + 1] == '(')
        {
            size_t endUrl = out.find(")", endAlt + 2);
            if (endUrl != std::string::npos)
            {
                std::string altText = out.substr(pos + 2, endAlt - (pos + 2));
                std::string url = out.substr(endAlt + 2, endUrl - (endAlt + 2));
                std::string imgTag = "<img src=\"" + url + "\" alt=\"" + altText + "\">";
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

std::string ParseMarkdown(const std::string& markdown)
{
    std::istringstream iss(markdown);
    std::string line;
    std::ostringstream html;
    bool inUl = false;
    bool inOl = false;
    while (std::getline(iss, line))
    {
        if (line.empty())
        {
            CloseLists(html, inUl, inOl);
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