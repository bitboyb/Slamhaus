#include <regex>
#include <sstream>
#include <iostream>
#include <cctype>
#include <vector>
#include <string>
#include "../parser/parser.hpp"
#include "../parser/text.hpp"
#include "../parser/attributes.hpp"

// TODO: (Assad) Make this readable, this is proper dog water.

namespace Script
{
    static std::string Sanitize(const std::string& s)
    {
        std::string out;
        out.reserve(s.size());
        for (char c : s)
        {
            if (std::isalnum(static_cast<unsigned char>(c)) || c == '_')
            {
                out.push_back(c);
            }
            else
            {
                out.push_back('_');
            }
        }
        if (!out.empty() && std::isdigit(static_cast<unsigned char>(out[0])))
        {
            out.insert(out.begin(), '_');
        }
        return out;
    }

    std::vector<std::string> ParseList(const std::string& csv)
    {
        std::vector<std::string> out;
        std::stringstream ss(csv);
        std::string item;
        while (std::getline(ss, item, ','))
        {
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
                        const std::string& returnType,
                        const std::string& result,
                        bool loop)
    {
        html << "<script type=\"module\">\n"
            << "import init from \"" << src << "\";\n"
            << "init().then(m => {\n";

        if (loop)
        {
            html << "  function updateLoop() {\n";
            html << "    const r = m.ccall(\"" << call << "\", \"" << returnType << "\", [], []);\n";
            html << "    document.getElementById(\"" << result << "\").textContent = r;\n";
            html << "    requestAnimationFrame(updateLoop);\n";
            html << "  }\n";
            html << "  updateLoop();\n";
        }
        else
        {
            if (!bind.empty()) 
            {
                html << "  const el = document.querySelector(\"" << bind << "\");\n"
                    << "  if (el) el.addEventListener(\"" << event << "\", () => {\n";
            } 
            else 
            {
                html << "  (() => {\n";
            }

            WriteInputCollection(html, argNames);

            html << "    const argTypes = [";
            for (size_t i = 0; i < argTypes.size(); ++i) 
            {
                html << "\"" << argTypes[i] << "\"";
                if (i + 1 < argTypes.size()) html << ", ";
            }
            html << "];\n";

            html << "    const argValues = [";
            for (size_t i = 0; i < argNames.size(); ++i) 
            {
                auto var = Sanitize(argNames[i]);
                html << "val_" << var;
                if (i + 1 < argNames.size()) html << ", ";
            }
            html << "];\n";

            bool isVoid = (returnType == "void") || result.empty();
            if (isVoid) 
            {
                html << "    m.ccall(\"" << call << "\", null, argTypes, argValues);\n";
            } 
            else 
            {
                html << "    const r = m.ccall(\"" << call << "\", \"" << returnType << "\", argTypes, argValues);\n"
                    << "    document.getElementById(\"" << result << "\").textContent = r;\n";
            }

            if (!bind.empty()) 
            {
                html << "  });\n";
            } 
            else 
            {
                html << "  })();\n";
            }
        }

        html << "});\n</script>\n";
    }

    bool HandleScript(const std::string& line,
                     std::istringstream& iss,
                     std::ostringstream& html,
                     Parser::ParseState& pState)
    {
        std::string trimmed = line;
        trimmed.erase(0, trimmed.find_first_not_of(" \t"));
        trimmed.erase(trimmed.find_last_not_of(" \t") + 1);
        static const std::regex blockRegex(R"(^@script\[(.*?)\]\(\)$)");
        std::smatch match;
        if (!std::regex_match(trimmed, match, blockRegex))
        {
            return false;
        }

        auto attrs    = Attributes::ParseAttributes(trimmed, "@script");
        const auto& src    = attrs["src"];
        const auto& call   = attrs["call"];
        const auto bind   = attrs.count("bind")  ? attrs.at("bind")  : std::string{};
        const auto event  = attrs.count("event") ? attrs.at("event") : std::string{"click"};
        const auto args   = attrs["args"];
        const auto types  = attrs["types"];
        const auto result = attrs["result"];
        const auto loop = attrs.count("loop") ? attrs.at("loop") == "true" : false;

    
        auto argNames = args.empty()
                        ? std::vector<std::string>{}
                        : ParseList(args);
        auto argTypes = types.empty()
                        ? std::vector<std::string>(argNames.size(), "string")
                        : ParseList(types);
        if (!ValidateArgumentMatch(argNames, argTypes))
        {
            return false;
        } 

        std::string returnType;

        if (attrs.count("rtype"))
        {
            returnType = attrs["rtype"];
        }
        else 
        {
            bool allNumeric = !argTypes.empty() && 
                              std::all_of(argTypes.begin(), argTypes.end(), 
                              [](auto &t){ return t=="int" || t=="float"; });
            returnType = allNumeric ? "number" : "string";
        }
    
        Text::CloseLists(html, pState);
        Text::CloseParagraphs(html);
        WriteGlueScript(html,
                          src,
                          call,
                          bind,
                          event,
                          argNames,
                          argTypes,
                          returnType,
                          result,
                          loop);

        return true;
    }
}
