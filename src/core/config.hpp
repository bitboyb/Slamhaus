#pragma once

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

namespace Config
{
    /// Represents values parsed from `config.ini`.
    /// Provides site-wide settings like site name, URL, description, and SEO defaults.
    struct ConfigINI
    {
        std::string siteName = "A Slamhaus Website!";
        std::string siteUrl = "https://nourl.dev";
        std::string description = "Default define.ini, please populate!";
        std::string faviconPath = "";
        std::string defaultKeywords = "";
    };

    /// Parses `content/include/config.ini` and returns a ConfigINI struct.
    /// - Falls back to default values if the file or specific keys are missing.
    ///
    /// @param contentDir Path to the root content directory
    /// @return A fully populated ConfigINI struct
    ConfigINI GetConfig(const std::string &contentDir);
}

#endif // CONFIG_HPP
