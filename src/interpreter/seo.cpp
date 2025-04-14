#include "seo.hpp"
#include <regex>
#include <map>

namespace SEO
{
    static std::string siteTitle;
    static std::string siteDescription;
    static std::string siteUrl;
    static std::string siteFavicon;
    static std::string pageDescription;
    static std::string pageKeywords;

    bool IsSiteLine(const std::string &line) { return line.find("$site[") == 0;}
    bool IsPageLine(const std::string &line) { return line.find("$page[") == 0; }
    std::string GetSiteFavicon() { return siteFavicon; }
    std::string GetSiteUrl()     { return siteUrl; }
    std::string GetSiteTitle() { return siteTitle; }
    std::string GetSiteDescription() { return siteDescription; }
    std::string GetPageDescription() { return pageDescription; }
    std::string GetPageKeywords() { return pageKeywords; }
    void SetSiteTitle(const std::string &title)           { siteTitle = title; }
    void SetSiteDescription(const std::string &desc)      { siteDescription = desc; }
    void SetSiteFavicon(const std::string &path)          { siteFavicon = path; }
    void SetSiteUrl(const std::string &url)               { siteUrl = url; }
    
    std::map<std::string, std::string> ParseAttributes(const std::string &line)
    {
        std::map<std::string, std::string> attributes;
        std::regex attrRegex(R"((\w+):\"([^\"]*)\")");
        std::smatch match;

        auto contentStart = line.find('[');
        auto contentEnd = line.find(']');
        if (contentStart == std::string::npos || contentEnd == std::string::npos)
            return attributes;

        std::string content = line.substr(contentStart + 1, contentEnd - contentStart - 1);
        auto it = content.cbegin();
        while (std::regex_search(it, content.cend(), match, attrRegex))
        {
            attributes[match[1]] = match[2];
            it = match.suffix().first;
        }
        return attributes;
    }

    std::string ProcessSiteLine(const std::string &line)
    {
        auto attrs = ParseAttributes(line);
        if (attrs.count("title")) 
        {
            siteTitle = attrs["title"];
        }
        if (attrs.count("description")) 
        {
            siteDescription = attrs["description"];
        }
        if (attrs.count("url")) 
        {
            siteUrl = attrs["url"];
        }
        if (attrs.count("favicon"))
        {
            siteFavicon = attrs["favicon"];
        }
        return "";
    }
    
    std::string ProcessPageLine(const std::string &line)
    {
        auto attrs = ParseAttributes(line);
        if (attrs.count("description")) 
        {
            pageDescription = attrs["description"];
        }
        if (attrs.count("keywords")) 
        {
            pageKeywords = attrs["keywords"];
        }
        return "";
    }

    std::string GetMetaTags()
    {
        std::ostringstream meta;
        if (!pageDescription.empty())
        {
            meta << "    <meta name=\"description\" content=\"" << pageDescription << "\">\n";
            meta << "    <meta property=\"og:description\" content=\"" << pageDescription << "\">\n";
        }
        if (!pageKeywords.empty())
        {
            meta << "    <meta name=\"keywords\" content=\"" << pageKeywords << "\">\n";
        }
        if (!siteTitle.empty())
        {
            meta << "    <meta property=\"og:title\" content=\"" << siteTitle << "\">\n";
        }
        if (!siteUrl.empty())
        {
            meta << "    <link rel=\"canonical\" href=\"" << siteUrl << "\">\n";
            meta << "    <meta property=\"og:url\" content=\"" << siteUrl << "\">\n";
        }
        return meta.str();
    }

    void Extract(const std::string &markdown)
    {
        std::istringstream stream(markdown);
        std::string line;
        while (std::getline(stream, line))
        {
            if (IsSiteLine(line))
            {
                ProcessSiteLine(line);
            }
            else if (IsPageLine(line))
            {
                ProcessPageLine(line);
            }
        }
    }

    void Reset()
    {
        siteTitle.clear();
        siteDescription.clear();
        pageDescription.clear();
        pageKeywords.clear();
        siteFavicon.clear();
        siteUrl.clear();
    }
}
