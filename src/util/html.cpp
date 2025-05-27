#include "html.hpp"
#include <regex>
#include <string>

namespace HTML
{
    std::string AdjustLinks(const std::string &html)
    {
        std::regex extRegex(R"(href="([^"]+)\.(md|smd|slam))");
        return std::regex_replace(html, extRegex, "href=\"$1.html");
    }

    std::string AdjustPath(const std::string &path)
    {
        size_t hashPos = path.find('#');
        std::string base = hashPos != std::string::npos ? path.substr(0, hashPos) : path;
        std::string fragment = hashPos != std::string::npos ? path.substr(hashPos) : "";

        std::regex extRegex(R"((\.md|\.smd|\.slam)$)");
        base = std::regex_replace(base, extRegex, ".html");

        return base + fragment;
    }
}
