#include "../parser/parser.hpp"
#include "../parser/seo.hpp"
#include "../parser/nav.hpp"
#include "config.hpp"
#include <fstream>
#include <iostream>

namespace Template
{    
    std::string BuildTemplate(Config::ConfigINI &ini)
    {
        auto readFile = [](const std::string &path) -> std::string 
        {
            std::ifstream in(path);
            if (!in) 
            {
                return "";
            }
            return std::string((std::istreambuf_iterator<char>(in)),
                               std::istreambuf_iterator<char>());
        };
    
        std::string headerMarkdown = readFile("content/include/header.md");
        std::string navMarkdown = readFile("content/include/nav.md");
        std::string footerMarkdown = readFile("content/include/footer.md");

        SEO::SEOData dummySEO = SEO::GetDefault(ini);
    
        std::string headerContent = Parser::ParseMarkdown(headerMarkdown, nullptr);
        std::string navContent = Nav::ParseNavigation(navMarkdown);
        std::string footerContent = Parser::ParseMarkdown(footerMarkdown, nullptr);
    
        std::string tmpl =
            "<!DOCTYPE html>\n"
            "<html>\n"
            "<head>\n"
            "    <meta charset=\"UTF-8\">\n"
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

