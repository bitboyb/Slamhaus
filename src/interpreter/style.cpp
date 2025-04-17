#include "style.hpp"
#include "id.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>
#include "../util/string.hpp"

namespace Style
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
}
