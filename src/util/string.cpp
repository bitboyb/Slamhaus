#include <algorithm>
#include <string>

namespace String
{
    std::string Trim(const std::string& str)
    {
        const auto strBegin = std::find_if_not(str.begin(), str.end(), ::isspace);
        const auto strEnd = std::find_if_not(str.rbegin(), str.rend(), ::isspace).base();
        return (strBegin < strEnd) ? std::string(strBegin, strEnd) : "";
    }
}