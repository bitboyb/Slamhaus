#include "form.hpp"
#include "text.hpp"
#include "attributes.hpp"
#include "../util/string.hpp"
#include <regex>
#include <map>

namespace Form
{
    std::map<std::string,std::string> ParseAttributes(const std::string &line)
    {
        std::map<std::string,std::string> attrs;
        size_t lb = line.find('['), rb = line.find(']');
        if (lb == std::string::npos || rb == std::string::npos || rb <= lb)
        {
            return attrs;
        }
        std::string inside = line.substr(lb+1, rb-lb-1);
        std::regex re(R"((\w+):\"([^\"]*)\")");
        std::smatch m;
        auto it = inside.cbegin();
        while (std::regex_search(it, inside.cend(), m, re)) 
        {
            attrs[m[1]] = m[2];
            it = m.suffix().first;
        }
        return attrs;
    }

    bool IsFormOpenLine(const std::string &line)
    {
        auto t = String::Trim(line);
        return t.rfind("?form[",0)==0 && t.find(']')!=std::string::npos;
    }

    bool IsFormCloseLine(const std::string &line)
    {
        return String::Trim(line) == "?/form[]()";
    }

    void ProcessFormOpen(const std::string & line,
                                std::ostringstream & html)
    {
        auto attrs = Attributes::ParseAttributes(line, "?form");

        std::ostringstream tagBuilder;
        tagBuilder << "<form";

        for (auto & [key, value] : attrs)
        {
            tagBuilder << " " << key << "=\"" << value << "\"";
        }

        tagBuilder << ">";
        html << tagBuilder.str() << "\n";
    }

    bool HandleFormElements(const std::string& line,
                            std::ostringstream& html,
                            Parser::ParseState& pState)
    {
        if (IsFormOpenLine(line)) 
        {
            Text::CloseLists(html, pState);
            ProcessFormOpen(line, html);
            pState.inForm = true;
            return true;
        }
        if (IsFormCloseLine(line)) 
        {
            html << "</form>\n";
            pState.inForm = false;
            return true;
        }
        return false;
    }
}
