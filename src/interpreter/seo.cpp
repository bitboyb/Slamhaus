#include "seo.hpp"
#include <regex>
#include <map>

namespace SEO
{
    SEO::SEOData GetDefault(Config::ConfigINI &ini)
    {
        SEO::SEOData seo;
        seo.siteTitle = ini.siteName;
        seo.siteDescription = ini.description;
        seo.siteUrl = ini.siteUrl;
        seo.siteFavicon = ini.faviconPath;
        seo.pageKeywords = ini.defaultKeywords;
        return seo;
    }

    bool IsSiteLine(const std::string &line) { return line.find("$site[") == 0;}
    bool IsPageLine(const std::string &line) { return line.find("$page[") == 0; }
    
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

    void ProcessSiteLine(const std::string &line, SEOData &seo)
    {
        auto attrs = ParseAttributes(line);
        if (attrs.count("title")) seo.siteTitle = attrs["title"];
        if (attrs.count("description")) seo.siteDescription = attrs["description"];
        if (attrs.count("url")) seo.siteUrl = attrs["url"];
        if (attrs.count("favicon")) seo.siteFavicon = attrs["favicon"];
    }
    
    void ProcessPageLine(const std::string &line, SEOData &seo)
    {
        auto attrs = ParseAttributes(line);
        if (attrs.count("description")) seo.pageDescription = attrs["description"];
        if (attrs.count("keywords")) seo.pageKeywords = attrs["keywords"];
    }

    std::string GetMetaTags(const SEOData &seo)
    {
        std::ostringstream meta;
        if (!seo.pageDescription.empty())
        {
            meta << "    <meta name=\"description\" content=\"" << seo.pageDescription << "\">\n";
            meta << "    <meta property=\"og:description\" content=\"" << seo.pageDescription << "\">\n";
        }
        if (!seo.pageKeywords.empty())
        {
            meta << "    <meta name=\"keywords\" content=\"" << seo.pageKeywords << "\">\n";
        }
        if (!seo.siteTitle.empty())
        {
            meta << "    <meta property=\"og:title\" content=\"" << seo.siteTitle << "\">\n";
        }
        if (!seo.siteUrl.empty())
        {
            meta << "    <link rel=\"canonical\" href=\"" << seo.siteUrl << "\">\n";
            meta << "    <meta property=\"og:url\" content=\"" << seo.siteUrl << "\">\n";
        }
        return meta.str();
    }

    bool HandleMetaLines(const std::string& line, 
                         SEO::SEOData* seo)
    {
        if (IsSiteLine(line)) 
        {
            ProcessSiteLine(line, *seo);
            return true;
        }
        if (IsPageLine(line)) 
        {
            ProcessPageLine(line, *seo);
            return true;
        }
        return false;
    }
}
