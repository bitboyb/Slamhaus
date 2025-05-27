#include "section.hpp"
#include "attributes.hpp"
#include "text.hpp"
#include "../util/string.hpp"

namespace Section
{
    bool HandleHeroOpen(const std::string &line,
                        std::ostringstream &html,
                        Parser::ParseState &pState)
    {
        std::string trimmed = String::Trim(line);
        if (trimmed.rfind(":hero[", 0) != 0) 
        {
            return false;
        }

        Text::CloseLists(html, pState);
        html << "<section class=\"hero\"";

        auto attrs = Attributes::ParseAttributes(line, ":hero");
        if (attrs.count("id")) 
        {
            html << " id=\"" << attrs["id"] << "\"";
        }
        html << ">\n";
        return true;
    }

    bool HandleHeroClose(const std::string &line,
                         std::ostringstream &html)
    {
        if (String::Trim(line) != ":/hero") 
        {
            return false;
        }

        html << "</section>\n";
        return true;
    }

    bool HandleSectionOpen(const std::string &line,
                           std::ostringstream &html,
                           Parser::ParseState &pState)
    {
        std::string trimmed = String::Trim(line);
        if (trimmed.rfind(":section[", 0) != 0) 
        {
            return false;
        }

        Text::CloseLists(html, pState);
        html << "<section";

        auto attrs = Attributes::ParseAttributes(line, ":section");
        if (attrs.count("id")) 
        {
            html << " id=\"" << attrs["id"] << "\"";
        }
        html << ">\n";
        return true;
    }

    bool HandleSectionClose(const std::string &line,
                            std::ostringstream &html)
    {
        if (String::Trim(line) != ":/section") 
        {
            return false;
        }

        html << "</section>\n";
        return true;
    }

    bool HandleColumnOpen(const std::string &line,
                          std::ostringstream &html,
                          Parser::ParseState &pState,
                          Parser::ColumnState &cState)
    {
        std::string trimmed = String::Trim(line);
        if (trimmed.rfind(":column[", 0) != 0) 
        {
            return false;
        }

        if (pState.inColumn && !cState.columnBuffer.str().empty()) 
        {
            cState.columnParts.push_back(cState.columnBuffer.str());
            cState.columnOpenLines.push_back(line);
            cState.columnBuffer.str("");
            cState.columnBuffer.clear();
        }
        else if (!pState.inColumn) 
        {
            Text::CloseLists(html, pState);
            pState.inColumn = true;
            cState.columnParts.clear();
            cState.columnOpenLines.clear();
            cState.columnBuffer.str("");
            cState.columnBuffer.clear();
            cState.columnOpenLines.push_back(line);
        }

        return true;
    }

    std::string ApplyColumns(int fallbackNumber,
                             const std::vector<std::string>& text,
                             const std::vector<std::string>& openLines)
    {
        std::ostringstream oss;
        int numColumns = fallbackNumber;

        if (!openLines.empty())
        {
            auto attrs = Attributes::ParseAttributes(openLines[0], ":column");
            if (attrs.count("size"))
            {
                try
                {
                    numColumns = std::stoi(attrs["size"]);
                }
                catch (...) {}
            }
        }

        oss << "<div class=\"columns-container responsive-columns\">";
        for (int i = 0; i < numColumns; i++)
        {
            std::string content = i < (int)text.size() ? text[i] : "";
            std::string openLine = i < (int)openLines.size() ? openLines[i] : "";

            oss << "<div class=\"column\"";
            auto attrs = Attributes::ParseAttributes(openLine, ":column");
            if (attrs.count("id"))
            {
                oss << " id=\"" << attrs["id"] << "\"";
            }
            oss << ">";
            oss << content << "</div>";
        }
        oss << "</div>";
        return oss.str();
    }


    bool HandleColumnClose(const std::string &line,
                           std::ostringstream &html,
                           Parser::ParseState &pState,
                           Parser::ColumnState &cState,
                           SEO::SEOData *seo)
    {
        if (String::Trim(line) != ":/column") 
        {
            return false;
        }

        if (!cState.columnBuffer.str().empty()) 
        {
            cState.columnParts.push_back(cState.columnBuffer.str());
        }
        for (auto & col : cState.columnParts) 
        {
            col = Parser::ParseMarkdown(col, seo);
        }
        html << ApplyColumns(
            static_cast<int>(cState.columnParts.size()),
            cState.columnParts,
            cState.columnOpenLines
        );
        pState.inColumn = false;
        cState.columnBuffer.str("");
        cState.columnBuffer.clear();
        return true;
    }

    bool HandleBlockElements(const std::string &line,
                             std::ostringstream &html,
                             Parser::ParseState &pState,
                             Parser::ColumnState &cState,
                             SEO::SEOData *seo)
    {
        if (HandleHeroOpen(line, html, pState)) 
        {
            return true;
        }
        if (HandleHeroClose(line, html)) 
        {
            return true;
        }
        if (HandleSectionOpen(line, html, pState))
        {
            return true;
        }
        if (HandleSectionClose(line, html)) 
        {
            return true;
        }
        if (HandleColumnOpen(line, html, pState, cState)) 
        {
            return true;
        }
        if (HandleColumnClose(line, html, pState, cState, seo)) 
        {
            return true;
        }
        if (pState.inColumn)
        {
            if (line == "---")
            {
                cState.columnParts.push_back(cState.columnBuffer.str());
                cState.columnBuffer.str("");
                cState.columnBuffer.clear();
            }
            else
            {
                cState.columnBuffer << line << "\n";
            }
            return true;
        }
        return false;
    }
}
