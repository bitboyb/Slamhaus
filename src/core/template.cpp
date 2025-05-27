#include "../parser/parser.hpp"
#include "../parser/seo.hpp"
#include "../parser/nav.hpp"
#include "config.hpp"
#include <fstream>
#include <iostream>

namespace Template
{    
    std::string BuildTemplate(Config::ConfigINI &ini,
                              const std::string contentDir)
    {
        auto readFile = [](const std::string &basePath) -> std::string
        {
            const std::vector<std::string> extensions = { ".md", ".smd", ".slam" };
            for (const auto &ext : extensions)
            {
                std::ifstream in(basePath + ext);
                if (in)
                {
                    return std::string((std::istreambuf_iterator<char>(in)),
                                    std::istreambuf_iterator<char>());
                }
            }
            return "";
        };

        std::string headerMarkdown = readFile(contentDir + "/include/header");
        std::string navMarkdown    = readFile(contentDir + "/include/nav");
        std::string footerMarkdown = readFile(contentDir + "/include/footer");

        SEO::SEOData dummySEO = SEO::GetDefault(ini);
    
        std::string headerContent = Parser::ParseMarkdown(headerMarkdown, nullptr);
        std::string navContent = Nav::ParseNavigation(navMarkdown);
        std::string footerContent = Parser::ParseMarkdown(footerMarkdown, nullptr);
    
        std::string tmpl =
            "<!DOCTYPE html>\n"
            "<html>\n"
            "<head>\n"
            "    <meta charset=\"UTF-8\">\n"
            "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
            "    <title>{{ title }}</title>\n"
            "    {{ favicon }}\n"
            "    {{ meta }}\n"
            "    <style>\n"
            "    {{ css }}\n"
            "    </style>\n"
            "</head>\n"
            "<body>\n"
            "    <header>\n"
            "       " + headerContent + "\n"
            "    </header>\n"
            "    " + navContent + "\n"
            "    <main>\n"
            "       {{ content }}\n"
            "    </main>\n"
            "    <footer>\n"
            "       " + footerContent + "\n"
            "    </footer>\n"
            "</body>\n"
            "</html>\n";
        return tmpl;
    }    
    
    std::string ApplyTemplateFromString(const std::string &content, 
                                        const std::string &templateString, 
                                        const std::string &cssContent,
                                        SEO::SEOData &seo)
    {
        std::string seoMeta = SEO::GetMetaTags(seo);
        std::string tmplStr = templateString;
        size_t pos = tmplStr.find("{{ css }}");
        if (pos != std::string::npos)
        {
            tmplStr.replace(pos, 9, cssContent);
        }
        pos = tmplStr.find("{{ content }}");
        if (pos != std::string::npos)
        {
            tmplStr.replace(pos, 13, content);
        }
        else
        {
            tmplStr += content;
        }
        pos = tmplStr.find("{{ title }}");
        if (pos != std::string::npos)
        {
            tmplStr.replace(pos, 11, seo.siteTitle);
        }
        pos = tmplStr.find("{{ favicon }}");
        if (pos != std::string::npos)
        {
            if (!seo.siteFavicon.empty())
            {
                std::string tag = "<link rel=\"icon\" type=\"image/x-icon\" href=\"" + seo.siteFavicon + "\">";
                tmplStr.replace(pos, 13, tag);
            }
            else
            {
                tmplStr.replace(pos, 13, "");
            }
        }
        pos = tmplStr.find("{{ meta }}");
        if (pos != std::string::npos)
        {
            tmplStr.replace(pos, 10, seoMeta);
        }
        return tmplStr;
    }
}

