#include "interaction.hpp"
#include "attributes.hpp"
#include "text.hpp"
#include "parser.hpp"
#include <set>
#include <sstream>
#include "../util/string.hpp"

namespace Interaction
{
    bool IsButtonLine(const std::string & line)
    {
        auto trimmed = String::Trim(line);
        return line.rfind("?button[", 0) == 0;
    }

    bool IsInputLine(const std::string & line)
    {
        auto trimmed = String::Trim(line);
        return line.rfind("?input[", 0) == 0;
    }

    bool IsTextareaLine(const std::string & line)
    {
        auto trimmed = String::Trim(line);
        return line.rfind("?textarea[", 0) == 0;
    }

    void EmitAttributes(std::ostringstream &html,
                        const std::map<std::string, 
                        std::string> & attrs)
    {
        static const std::set<std::string> boolAttrs = 
        {
            "required",
            "checked", 
            "disabled", 
            "multiple", 
            "autofocus"
        };

        for (auto const & kv : attrs)
        {
            auto const & key   = kv.first;
            auto const & value = kv.second;

            if (boolAttrs.count(key))
            {
                html << " " << key;
            }
            else if (!value.empty())
            {
                html << " " << key << "=\"" << value << "\"";
            }
        }
    }

    std::string ProcessButton(const std::string &line,
                              Parser::ParseState &pState)
    {
        auto trimmed = String::Trim(line);
        auto attrs   = Attributes::ParseAttributes(trimmed, "?button");
        if (!attrs.count("type"))
        {
            attrs["type"] = "submit";
        }
        std::string action = attrs.count("action") ? attrs["action"] : "";
        std::string label = attrs.count("text") ? attrs["text"] : "Submit";
        std::string id = attrs.count("id") ? attrs["id"] : "";

        std::ostringstream html;
        html << "<button type=\""
             << (action.empty() ? "submit\"" : "button\"");

        if (!action.empty())
        {
            if (action.rfind("mailto:", 0) == 0 ||
                action.rfind("http://", 0) == 0 ||
                action.rfind("https://",0) == 0 ||
                action.rfind("/",      0) == 0 ||
                action.find(".html")  != std::string::npos)
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

        html << ">"
             << label
             << "</button>";

        return html.str();
    }

    std::string ProcessInput(const std::string & line,
                             Parser::ParseState  & pState)
    {
        auto trimmed = String::Trim(line);
        auto attrs   = Attributes::ParseAttributes(trimmed, "?input");
        std::ostringstream html;
        html << "<p><input";
        EmitAttributes(html, attrs);
        html << " /></p>";
        return html.str();
    }

    std::string ProcessTextarea(const std::string &line,
                                Parser::ParseState &pState)
    {
        auto trimmed = String::Trim(line);
        auto attrs   = Attributes::ParseAttributes(trimmed, "?textarea");
        std::ostringstream html;
        html << "<p><textarea";
        EmitAttributes(html, attrs);
        html << "></textarea></p>";
        return html.str();
    }

    bool IsInteraction(const std::string &line,
                       std::ostringstream &html,
                       Parser::ParseState &pState)
    {
        if (IsButtonLine(line))
        {
            Text::CloseLists(html, pState);
            html << ProcessButton(line, pState) << "\n";
            return true;
        }
        if (IsInputLine(line))
        {
            Text::CloseLists(html, pState);
            html << ProcessInput(line, pState) << "\n";
            return true;
        }
        if (IsTextareaLine(line))
        {
            Text::CloseLists(html, pState);
            html << ProcessTextarea(line, pState) << "\n";
            return true;
        }

        return false;
    }
}
