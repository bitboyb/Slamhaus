#pragma once
#include <string>

#ifndef CONFIG_HPP
#define CONFIG_HPP

namespace Config
{
    struct ConfigINI
    {
        std::string siteName = "A Slamhaus Website!";
        std::string siteUrl = "https://nourl.dev";
        std::string description = "Default define.ini, please populate!";
        std::string faviconPath = "";
    };

    ConfigINI GetConfig();
}

#endif // CONFIG_HPP
