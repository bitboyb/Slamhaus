#include "id.hpp"
#include <string>
#include <regex>

namespace ID 
{
    std::string ExtractIDTag(const std::string& line)
    {
        std::regex id_pattern(R"(\(#([a-zA-Z0-9\-_]+)\))");
        std::smatch match;
        if (std::regex_search(line, match, id_pattern) && match.size() > 1) {
            return match[1].str();
        }
        return "";
    }
    
    std::string ApplyIDTag(const std::string& htmlLine, const std::string& markdownLine)
    {
        std::string id = ExtractIDTag(markdownLine);
        if (id.empty()) return htmlLine;
    
        std::string result = htmlLine;
    
        size_t tag_end = result.find('>');
        if (tag_end == std::string::npos) return htmlLine;
    
        if (result.find("id=") != std::string::npos) return htmlLine;
    
        size_t insert_pos = result.find(' ');
        if (insert_pos == std::string::npos || insert_pos > tag_end) {
            insert_pos = tag_end;
        }
    
        result.insert(insert_pos, " id=\"" + id + "\"");
        return result;
    }
    
    bool isIDTag(const std::string& line)
    {
        return !ExtractIDTag(line).empty();
    }    
    
}
