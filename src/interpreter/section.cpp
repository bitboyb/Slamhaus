#include "section.hpp"
#include "id.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>
#include "parser.hpp"
#include "text.hpp"
#include "../util/string.hpp"

namespace Section
{
    std::string ApplyColumns(int number, 
                             const std::vector<std::string>& text, 
                             const std::vector<std::string>& openLines)
    {
        std::ostringstream oss;
        oss << "<div class=\"columns-container responsive-columns\">";
        for (int i = 0; i < number; i++)
        {
            std::string content = (i < static_cast<int>(text.size()) ? text[i] : "");
            std::string openLine = (i < static_cast<int>(openLines.size()) ? openLines[i] : "");
    
            std::string colTag = "<div class=\"column\">";
            if (ID::isIDTag(openLine))
            {
                colTag = ID::ApplyIDTag(colTag, openLine);
            }
            oss << colTag;
            oss << content;
            oss << "</div>";
        }
        oss << "</div>";
        return oss.str();
    }    

    std::string ApplyHero(const std::string &content, const std::string &openLine)
    {
        std::string div = "<div class=\"hero-section\">";
        if (ID::isIDTag(openLine))
        {
            div = ID::ApplyIDTag(div, openLine);
        }

        return div + content + "</div>";
    }

    bool IsColumnOpenLine(const std::string &line)
    {
        std::string trimmed = String::Trim(line);
        return trimmed.rfind(":column[", 0) == 0;
    }

    bool IsColumnCloseLine(const std::string &line)
    {
        return String::Trim(line) == ":/column";
    }

    bool IsSectionOpenLine(const std::string &line)
    {
        std::string trimmed = String::Trim(line);
        return trimmed.rfind(":section[", 0) == 0;
    }

    bool IsSectionCloseLine(const std::string &line)
    {
        return String::Trim(line) == ":/section";
    }

    bool IsHeroOpenLine(const std::string &line)
    {
        std::string trimmed = String::Trim(line);
        return trimmed.rfind(":hero[", 0) == 0;
    }

    bool IsHeroCloseLine(const std::string &line)
    {
        return String::Trim(line) == ":/hero";
    }

    bool HandleBlockElements(const std::string& line,
                             std::ostringstream& html,
                             Parser::ParseState& pState,
                             Parser::ColumnState& cState,
                             SEO::SEOData* seo)
    {
        if (IsHeroOpenLine(line)) 
        {
            Text::CloseLists(html, pState);
            std::string htmlLine = "<section class=\"hero\">";
            if (ID::isIDTag(line)) 
            {
                htmlLine = ID::ApplyIDTag(htmlLine, line);
            }
            html << htmlLine << "\n";
            return true;
        }
        if (IsHeroCloseLine(line)) 
        {
            html << "</section>\n";
            return true;
        }
        if (IsSectionOpenLine(line)) 
        {
            Text::CloseLists(html, pState);
            std::string htmlLine = "<section>";
            if (ID::isIDTag(line)) {
                htmlLine = ID::ApplyIDTag(htmlLine, line);
            }
            html << htmlLine << "\n";
            return true;
        }
        if (IsSectionCloseLine(line)) 
        {
            html << "</section>\n";
            return true;
        }
        if (IsColumnOpenLine(line)) 
        {
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
        if (IsColumnCloseLine(line)) 
        {
            if (!cState.columnBuffer.str().empty()) 
            {
                cState.columnParts.push_back(cState.columnBuffer.str());
            }
            for (auto& col : cState.columnParts) 
            {
                col = Parser::ParseMarkdown(col, seo);
            }
            html << ApplyColumns(static_cast<int>(cState.columnParts.size()), cState.columnParts, cState.columnOpenLines);
            pState.inColumn = false;
            cState.columnBuffer.str("");
            cState.columnBuffer.clear();
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
