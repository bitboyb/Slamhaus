#include "style.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace Style
{
    std::string ApplyColumns(int number, const std::vector<std::string>& text)
    {
        std::ostringstream oss;
        oss << "<div class=\"columns-container responsive-columns\">";
        for (int i = 0; i < number; i++)
        {
            std::string content = (i < static_cast<int>(text.size()) ? text[i] : "");
            oss << "<div class=\"column\">";
            oss << content;
            oss << "</div>";
        }
        oss << "</div>";
        return oss.str();
    }
    
    std::string ApplyStyle(std::string &text, std::string &css)
    {
        std::ostringstream oss;
        oss << "<div style=\"" << css << "\">" << text << "</div>";
        return oss.str();
    }

    std::string Trim(const std::string& str)
    {
        const auto strBegin = std::find_if_not(str.begin(), str.end(), ::isspace);
        const auto strEnd = std::find_if_not(str.rbegin(), str.rend(), ::isspace).base();
        return (strBegin < strEnd) ? std::string(strBegin, strEnd) : "";
    }

    bool IsColumnOpenLine(const std::string &line)
    {
        std::string trimmed = Trim(line);
        return trimmed.rfind(":column[", 0) == 0;
    }

    bool IsColumnCloseLine(const std::string &line)
    {
        return Trim(line) == ":/column";
    }

    bool IsSectionOpenLine(const std::string &line)
    {
        std::string trimmed = Trim(line);
        return trimmed.rfind(":section[", 0) == 0;
    }

    bool IsSectionCloseLine(const std::string &line)
    {
        return Trim(line) == ":/section";
    }
}
