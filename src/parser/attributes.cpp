#include "attributes.hpp"
#include "../util/html.hpp"

#include <regex>

namespace Attributes
{
    std::string ExtractIDTag(const std::string & line)
    {
        static const std::regex idPattern{ R"(\(#([a-zA-Z0-9\-_]+)\))" };
        std::smatch match;
        if (std::regex_search(line, match, idPattern) && match.size() > 1)
        {
            return match[1].str();
        }
        return std::string();
    }

    std::map<std::string, std::string> ParseAttributes(const std::string & line,
                                                       const std::string & prefix)
    {
        std::map<std::string, std::string> attrs;
        size_t start = line.find(prefix);
        if (start == std::string::npos)
        {
            return attrs;
        }

        size_t open  = line.find('[', start);
        size_t close = line.find(']', open + 1);
        if (open == std::string::npos || close == std::string::npos)
        {
            return attrs;
        }

        std::string body = line.substr(open + 1, close - open - 1);
        static const std::regex kv{ R"attr((\w+)\s*:\s*"([^"]*)")attr" };
        std::smatch m;
        auto it = body.cbegin();

        while (std::regex_search(it, body.cend(), m, kv))
        {
            std::string key = m[1].str();
            std::string val = m[2].str();

            if (key == "action" || key == "href" || key == "src" || key == "link")
            {
                val = HTML::AdjustPath(val);
            }

            attrs[key] = val;
            it = m.suffix().first;
        }

        std::string id = ExtractIDTag(line);
        if (!id.empty())
        {
            attrs["id"] = id;
        }

        return attrs;
    }
}
