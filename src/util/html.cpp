#include <regex>
#include <string>

namespace HTML
{
    std::string AdjustLinks(const std::string &html)
    {
        std::regex mdLinkRegex(R"(href="([^"]+)\.md")");
        return std::regex_replace(html, mdLinkRegex, "href=\"$1.html\"");
    }
}