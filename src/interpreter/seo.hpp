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

    bool IsSiteLine(const std::string &line);
    bool IsPageLine(const std::string &line);

    void ProcessSiteLine(const std::string &line, SEOData &seo);
    void ProcessPageLine(const std::string &line, SEOData &seo);    

    SEO::SEOData GetDefault(Config::ConfigINI &ini);
    std::string GetMetaTags(const SEOData &seo);
}

#endif // SEO_HPP
