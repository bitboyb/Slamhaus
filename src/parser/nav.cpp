#include <string>
#include <sstream>
#include <vector>
#include "../util/html.hpp"

namespace Nav
{
    std::string ParseNavigation(const std::string& navMarkdown)
    {
        std::istringstream stream(navMarkdown);
        std::string line;
        std::string html;
        std::vector<size_t> indentStack;
        bool firstItem = true;

        html += "<ul>\n";
        while (std::getline(stream, line))
        {
            size_t indent = line.find_first_not_of(" ");
            if (indent == std::string::npos) continue;
            size_t depth = indent / 4;

            while (indentStack.size() > depth)
            {
                html += "</li>\n</ul>\n";
                indentStack.pop_back();
            }

            if (line.find("- [") != std::string::npos)
            {
                size_t linkStart = line.find("[") + 1;
                size_t linkEnd = line.find("]");
                size_t hrefStart = line.find("(", linkEnd) + 1;
                size_t hrefEnd = line.find(")", hrefStart);
                if (linkEnd == std::string::npos || hrefStart == std::string::npos || hrefEnd == std::string::npos) continue;

                std::string text = line.substr(linkStart, linkEnd - linkStart);
                std::string href = line.substr(hrefStart, hrefEnd - hrefStart);
                
                href = HTML::AdjustPath(href);

                if (!firstItem && indentStack.size() == depth)
                {
                    html += "</li>\n";
                }
                if (indentStack.size() < depth)
                {
                    html += "\n<ul>\n";
                    indentStack.push_back(depth);
                }

                html += "<li><a href=\"" + href + "\">" + text + "</a>";
                firstItem = false;
            }
        }

        while (!indentStack.empty())
        {
            html += "</li>\n</ul>\n";
            indentStack.pop_back();
        }

        html += "</li>\n</ul>\n";
        return "<nav>\n"
               "  <input type=\"checkbox\" id=\"nav-toggle\" class=\"nav-toggle\" />\n"
               "  <label for=\"nav-toggle\" class=\"nav-toggle-label\">\n"
               "    <span></span>\n"
               "  </label>\n"
               "  <div class=\"nav-menu\">\n" + html +
               "  </div>\n"
               "</nav>\n";
    }
}
