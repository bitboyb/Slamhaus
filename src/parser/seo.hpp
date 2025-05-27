#pragma once

#ifndef SEO_HPP
#define SEO_HPP

#include <string>
#include "../core/config.hpp"

namespace SEO
{
    /// Holds global and page-level SEO metadata.
    /// Populated from $site[] and $page[] tags in Markdown.
    struct SEOData
    {
        std::string siteTitle;        ///< Title of the website (from config or $site[])
        std::string siteDescription;  ///< Global site description (from config or $site[])
        std::string siteUrl;          ///< Canonical URL (from config or $site[])
        std::string siteFavicon;      ///< Path to favicon (from config or $site[])
        std::string pageDescription;  ///< Per-page description (from $page[])
        std::string pageKeywords;     ///< Per-page keywords (from $page[])
    };

    /// Handles $site[] and $page[] declarations from Markdown source.
    /// Updates the given SEOData object if a match is found.
    ///
    /// @param line The raw line of Markdown
    /// @param seo  Pointer to SEOData to update
    /// @return True if the line was a valid SEO metadata declaration
    bool HandleMetaLines(const std::string& line, 
                         SEO::SEOData* seo); 

    /// Returns an SEOData object populated with defaults from config.ini.
    /// Used to initialize state before parsing a page.
    ///
    /// @param ini The parsed global config
    /// @return A default SEOData struct
    SEO::SEOData GetDefault(Config::ConfigINI &ini);
    
    /// Generates the HTML <meta> tags for OpenGraph, description, keywords, and canonical URL.
    ///
    /// @param seo The current SEO data for the page
    /// @return A block of HTML <meta> tags
    std::string GetMetaTags(const SEOData &seo);
}

#endif // SEO_HPP
