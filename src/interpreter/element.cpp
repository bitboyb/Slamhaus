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

    bool IsCodeLine(const std::string &line)
    {
        return line.rfind("?code[", 0) == 0 && !line.empty() && line.back() == ']';
    }

    bool IsButtonLine(const std::string &line)
    {
        return line.rfind("?button[", 0) == 0 && !line.empty() && line.back() == ']';
    }

    bool IsInputLine(const std::string &line)
    {
        return line.rfind("?input[", 0) == 0 && !line.empty() && line.back() == ']';
    }

    bool IsFormLine(const std::string &line)
    {
        return line.rfind("?form[", 0) == 0 && !line.empty() && line.back() == ']';
    }

    bool IsCodeCloseLine(const std::string &line)
    {
        return Trim(line) == "?/code";
    }

    bool IsFormCloseLine(const std::string &line)
    {
        return Trim(line) == "?/form";
    }

    std::string ProcessCode(const std::string &input)
    {
        
    }

    std::string ProcessButton(const std::string &input)
    {

    }

    std::string ProcessInputLine(const std::string &input)
    {

    }

    std::string ProcessFormLine(const std::string &input)
    {

    }
}
