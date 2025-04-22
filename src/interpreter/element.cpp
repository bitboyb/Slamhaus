#include "element.hpp"
#include "id.hpp"

#include <regex>
#include <sstream>
#include "parser.hpp"
#include "text.hpp"

namespace Element
{
    bool IsFormOpenLine(const std::string &line)
    {
        return false;
    }
    bool IsFormCloseLine(const std::string &line)
    {
        return false;
    }
    bool IsButtonLine(const std::string &line)
    {
        return line.find("?button[") == 0;
    }

    std::string ProcessButton(const std::string &line)
    {
        std::regex pattern(R"(\?button\[(?:\s*action:\"([^\"]*)\")?(?:\s+text:\"([^\"]*)\")?\](\(#([a-zA-Z0-9\-_]+)\))?)");
        std::smatch match;
        if (!std::regex_match(line, match, pattern)) 
        {
            return "<!-- Invalid button syntax -->";
        }
    
        std::string action = match[1].matched ? match[1].str() : "";
        std::string label = match[2].matched ? match[2].str() : "Submit";
        std::string id = match[4].matched ? match[4].str() : "";
    
        std::ostringstream html;
    
        html << "<button type=\"";
    
        if (action.empty())
        {
            html << "submit\"";
        }
        else
        {
            html << "button\"";
        
            if (action.find("mailto:") == 0 ||
                action.find("http://") == 0 ||
                action.find("https://") == 0 ||
                action.find("/") == 0 ||
                action.find(".html") != std::string::npos)
            {
                html << " onclick=\"window.location.href='" << action << "'\"";
            }
            else
            {
                html << " onclick=\"" << action << "()\"";
            }
        }        
        if (!id.empty())
        {
            html << " id=\"" << id << "\"";
        }
    
        html << ">" << label << "</button>";
        return html.str();
    }     

    bool HandleElementLines(const std::string& line, 
                            std::ostringstream& html, 
                            Parser::ParseState& pState)
    {
        if (IsButtonLine(line)) 
        {
            Text::CloseLists(html, pState);
            html << Element::ProcessButton(line) << "\n";
            return true;
        }
        if (IsFormOpenLine(line) || IsFormCloseLine(line)) 
        {
            return true;
        }
        return false;
    }
}
