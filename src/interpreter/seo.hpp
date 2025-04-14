#ifndef SEO_HPP
#define SEO_HPP

#include <string>

namespace SEO
{
    bool IsSiteLine(const std::string &line);
    bool IsPageLine(const std::string &line);

    std::string ProcessSiteLine(const std::string &line);
    std::string ProcessPageLine(const std::string &line);
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
