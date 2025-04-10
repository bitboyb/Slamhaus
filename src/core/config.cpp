#include "config.hpp"
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>
#include <cctype>

namespace Config
{
    std::string Trim(const std::string &str)
    {
        const auto strBegin = std::find_if_not(str.begin(), str.end(), ::isspace);
        const auto strEnd = std::find_if_not(str.rbegin(), str.rend(), ::isspace).base();
        return (strBegin < strEnd) ? std::string(strBegin, strEnd) : "";
    }

    std::map<std::string, std::string> ParseSimpleConfig(const std::string &path)
    {
        std::ifstream in(path);
        std::map<std::string, std::string> config;
        std::string line;

        while (std::getline(in, line))
        {
            if (line.empty() || line[0] == '#') continue;
            size_t eq = line.find('=');
            if (eq != std::string::npos)
            {
                std::string key = Trim(line.substr(0, eq));
                std::string val = Trim(line.substr(eq + 1));
                config[key] = val;
            }
        }
        return config;
    }

    ConfigINI GetConfig()
    {
        ConfigINI ini;
        const std::string iniPath = "content/include/config.ini";
        std::map<std::string, std::string> config = ParseSimpleConfig(iniPath);

        if (config.count("site_name")) ini.siteName = config["site_name"];
        if (config.count("site_url")) ini.siteUrl = config["site_url"];
        if (config.count("description")) ini.description = config["description"];
        if (config.count("favicon_path")) ini.faviconPath = config["favicon_path"];

        return ini;
    }
}
