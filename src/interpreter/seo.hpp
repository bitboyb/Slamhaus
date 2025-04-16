#ifndef SEO_HPP
#define SEO_HPP

#include <string>
#include "../core/config.hpp"

namespace SEO
{
    struct SEOData
    {
        std::string siteTitle;
        std::string siteDescription;
        std::string siteUrl;
        std::string siteFavicon;
        std::string pageDescription;
        std::string pageKeywords;
    };

    static std::string siteTitle;
    static std::string siteDescription;
    static std::string siteUrl;
    static std::string siteFavicon;
    static std::string pageDescription;
    static std::string pageKeywords;

    bool IsSiteLine(const std::string &line);
    bool IsPageLine(const std::string &line);

    std::string ProcessSiteLine(const std::string &line);
    std::string ProcessPageLine(const std::string &line);

    SEO::SEOData GetDefault(Config::ConfigINI &ini);

    // Delete this shit

    std::string GetSiteFavicon();
    std::string GetSiteUrl();
    void SetSiteTitle(const std::string &title);
    void SetSiteDescription(const std::string &desc);
    void SetSiteFavicon(const std::string &path);
    void SetSiteUrl(const std::string &url);
    std::string GetSiteTitle();
    std::string GetSiteDescription();
    std::string GetPageDescription();
    std::string GetPageKeywords();
    std::string GetMetaTags();

    void Extract(const std::string &markdown);
    void Reset();
}

#endif // SEO_HPP
