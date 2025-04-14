#include "element.hpp"
#include <algorithm>

#include <sstream>

namespace Element
{
    std::string Trim(const std::string& str)
    {
        const auto strBegin = std::find_if_not(str.begin(), str.end(), ::isspace);
        const auto strEnd = std::find_if_not(str.rbegin(), str.rend(), ::isspace).base();
        return (strBegin < strEnd) ? std::string(strBegin, strEnd) : "";
    }
    
    bool IsScriptOpenLine(const std::string &line)
    {
        return line.rfind("?code[", 0) == 0 && !line.empty() && line.back() == ']';
    }

    bool IsScriptCloseLine(const std::string &line)
    {
        return Trim(line) == "?/code";
    }

    bool IsFormOpenLine(const std::string &line)
    {
        return line.rfind("?form[", 0) == 0 && !line.empty() && line.back() == ']';
    }

    bool IsFormCloseLine(const std::string &line)
    {
        return Trim(line) == "?/form";
    }

    bool IsButtonLine(const std::string &line)
    {
        return line.rfind("?button[", 0) == 0 && !line.empty() && line.back() == ']';
    }

    bool IsInputLine(const std::string &line)
    {
        return line.rfind("?input[", 0) == 0 && !line.empty() && line.back() == ']';
    }
}
