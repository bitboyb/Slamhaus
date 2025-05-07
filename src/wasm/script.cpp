#include <regex>
#include <sstream>
#include <iostream>
#include <cctype>
#include <vector>
#include <string>
#include "../parser/parser.hpp"
#include "../parser/text.hpp"
#include "../parser/attributes.hpp"

namespace Script
{
    static std::string Sanitize(const std::string& s)
    {
        std::string out;
        out.reserve(s.size());
        for (char c : s)
        {
            if (std::isalnum(static_cast<unsigned char>(c)) || c == '_')
                out.push_back(c);
            else
                out.push_back('_');
        }
        if (!out.empty() && std::isdigit(static_cast<unsigned char>(out[0])))
            out.insert(out.begin(), '_');
        return out;
    }

    std::vector<std::string> ParseList(const std::string& csv)
    {
        std::vector<std::string> out;
        std::stringstream ss(csv);
        std::string item;
        while (std::getline(ss, item, ','))
        {
            // trim whitespace
            item.erase(0, item.find_first_not_of(" \t"));
            item.erase(item.find_last_not_of(" \t") + 1);
            out.push_back(item);
        }
        return out;
    }

    void WriteInputCollection(std::ostringstream& html,
                              const std::vector<std::string>& argNames)
    {
        for (auto const& arg : argNames)
        {
            std::string var = Sanitize(arg);
            html << "    const val_" << var
                 << " = document.getElementById(\"" << arg << "\")?.value || \"\";\n";
        }
    }

    bool ValidateArgumentMatch(const std::vector<std::string>& argNames,
                               const std::vector<std::string>& argTypes)
    {
        if (!argNames.empty() && argNames.size() != argTypes.size())
        {
            std::cerr << "Mismatch between args and types in @script.\n";
            return false;
        }
        return true;
    }

    void WriteGlueScript(std::ostringstream& html,
                        const std::string& src,
                        const std::string& call,
                        const std::string& bind,
                        const std::string& event,
                        const std::vector<std::string>& argNames,
                        const std::vector<std::string>& argTypes,
                        const std::string& result)
    {
        // if result is empty, we pass null as the returnType to ccall
        std::string returnType = result.empty() ? "null" : "\"string\"";

        html << "<script type=\"module\">\n";
        html << "import init from \"" << src << "\";\n";
        html << "init().then(m => {\n";

        if (!bind.empty())
        {
            // bind case: attach to an existing element
            html << "  const el = document.querySelector(\"" << bind << "\");\n"
                << "  if (el) el.addEventListener(\"" << event << "\", () => {\n";
        }
        else
        {
            // no bind: run immediately in an IIFE
            html << "  ((): void => {\n";
        }

        // 1) pull in the inputs
        WriteInputCollection(html, argNames);

        // 2) build the types array
        html << "    const argTypes = [";
        for (size_t i = 0; i < argTypes.size(); ++i)
        {
            html << "\"" << argTypes[i] << "\"";
            if (i + 1 < argTypes.size()) html << ", ";
        }
        html << "];\n";

        // 3) build the values array
        html << "    const argValues = [";
        for (size_t i = 0; i < argNames.size(); ++i)
        {
            auto var = Sanitize(argNames[i]);
            html << "val_" << var;
            if (i + 1 < argNames.size()) html << ", ";
        }
        html << "];\n";

        // 4) call into your WASM
        html << "    const r = m.ccall(\"" << call
            << "\", " << returnType
            << ", argTypes, argValues);\n";

        if (!result.empty())
        {
            html << "    document.getElementById(\"" << result
                << "\").textContent = r;\n";
        }

        // 5) close off the two cases
        if (!bind.empty())
        {
            // close the callback
            html << "  });\n";
        }
        else
        {
            // close the IIFE
            html << "  })();\n";
        }

        // 6) close the .then(…) and the module script tag
        html << "});\n";
        html << "</script>\n";
    }


    bool HandleScript(const std::string& line,
                      std::istringstream& /*iss*/,
                      std::ostringstream& html,
                      Parser::ParseState& pState)
    {
        // trim whitespace
        std::string trimmed = line;
        trimmed.erase(0, trimmed.find_first_not_of(" \t"));
        trimmed.erase(trimmed.find_last_not_of(" \t") + 1);

        static const std::regex blockRegex(R"(^@script\[(.*?)\]\(\)$)");
        std::smatch match;
        if (!std::regex_match(trimmed, match, blockRegex))
            return false;

        // parse *all* attributes from the full @script[... ]() line
        auto attrs = Attributes::ParseAttributes(trimmed, "@script");

        const auto& src    = attrs["src"];
        const auto& call   = attrs["call"];
        const auto  bind   = attrs.count("bind")   ? attrs.at("bind")  : std::string{};
        const auto  event  = attrs.count("event")  ? attrs.at("event") : "click";
        const auto  args   = attrs["args"];
        const auto  types  = attrs["types"];
        const auto  result = attrs["result"];

        if (src.empty() || call.empty())
        {
            std::cerr << "Missing required attributes in @script: src and call\n";
            return false;
        }

        Text::CloseLists(html, pState);
        Text::CloseParagraphs(html);

        auto argNames = args.empty() ? std::vector<std::string>{} : ParseList(args);
        auto argTypes = types.empty()
                      ? std::vector<std::string>(argNames.size(), "string")
                      : ParseList(types);

        if (!ValidateArgumentMatch(argNames, argTypes))
            return false;

        WriteGlueScript(html, src, call, bind, event, argNames, argTypes, result);
        return true;
    }
}
