#include "config.hpp"
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>
#include <cctype>
#include "../util/string.hpp"

namespace Config
{
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
                std::string key = String::Trim(line.substr(0, eq));
                std::string val = String::Trim(line.substr(eq + 1));
                config[key] = val;
            }
        }
        return config;
    }

    ConfigINI GetConfig(const std::string &contentDir)
    {
        ConfigINI ini;
        const std::string iniPath = contentDir + "/include/config.ini";
        std::map<std::string, std::string> config = ParseSimpleConfig(iniPath);

        if (config.count("site_name")) ini.siteName = config["site_name"];
        if (config.count("site_url")) ini.siteUrl = config["site_url"];
        if (config.count("description")) ini.description = config["description"];
        if (config.count("favicon_path")) ini.faviconPath = config["favicon_path"];
        if (config.count("default_keywords")) ini.defaultKeywords = config["default_keywords"];        

        return ini;
    }
}
