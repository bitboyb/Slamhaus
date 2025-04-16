#include "generator.hpp"
#include "../interpreter/parser.hpp"
#include "../interpreter/seo.hpp"
#include "nav.hpp"
#include "asset.hpp"
#include "config.hpp"
#include "meta.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>

namespace Generator 
{
    std::string AdjustLinks(const std::string &html)
    {
        std::regex mdLinkRegex(R"(href="([^"]+)\.md")");
        return std::regex_replace(html, mdLinkRegex, "href=\"$1.html\"");
    }
    
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
        std::string siteName = ini.siteName;
        std::string headerContent = Parser::ParseMarkdown(headerMarkdown);
        std::string navContent = Nav::ParseNavigation(navMarkdown);
        std::string footerContent = Parser::ParseMarkdown(footerMarkdown);
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
                                        const std::string &faviconPath,
                                        const std::string &seoMeta)
    {
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
            tmplStr.replace(pos, 11, SEO::GetSiteTitle());
        }
        pos = tmplStr.find("{{ favicon }}");
        if (pos != std::string::npos)
        {
            if (!faviconPath.empty())
            {
                std::string tag = "<link rel=\"icon\" type=\"image/x-icon\" href=\"" + faviconPath + "\">";
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

    std::string LoadCSS(const std::string &cssPath)
    {
        std::ifstream cssFile(cssPath);
        if (!cssFile)
        {
            std::cerr << "Error opening CSS file: " << cssPath << std::endl;
            return "";
        }
        std::ostringstream cssStream;
        cssStream << cssFile.rdbuf();
        return cssStream.str();
    }
    
    void GenerateSite(const std::string &contentDir, 
                      const std::string &outputDir, 
                      const std::string &cssPath)
    {
        std::filesystem::create_directories(outputDir);
        std::string cssContent = LoadCSS(cssPath);
        Config::ConfigINI ini = Config::GetConfig();
        std::vector<std::string> generatedPages;
        std::string templateString = BuildTemplate(ini);
        Asset::CopyAssets("content/assets/", outputDir + "/assets");
        for (const auto &entry : std::filesystem::recursive_directory_iterator(contentDir))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".md")
            {
                std::string filename = entry.path().filename().string();
                if (filename == "header.md" || filename == "nav.md"    || filename == "footer.md")
                {
                    std::cout << "Skipping partial file: " << entry.path() << std::endl;
                    continue;
                }
                std::filesystem::path relPath = std::filesystem::relative(entry.path(), contentDir);
                relPath.replace_extension(".html");
                std::filesystem::path outputPath = std::filesystem::path(outputDir) / relPath;
                std::filesystem::create_directories(outputPath.parent_path());
                std::ifstream mdFile(entry.path());
                if (!mdFile)
                {
                    std::cerr << "Error opening Markdown file: " << entry.path() << std::endl;
                    continue;
                }
                std::string markdown((std::istreambuf_iterator<char>(mdFile)),
                                     std::istreambuf_iterator<char>());
    
                std::string htmlContent = Parser::ParseMarkdown(markdown);
                SEO::Reset();
                SEO::Extract(markdown);
                if (SEO::GetSiteTitle().empty()) 
                {
                    SEO::SetSiteTitle(ini.siteName);
                }
                if (SEO::GetSiteDescription().empty()) 
                {
                    SEO::SetSiteDescription(ini.description);
                }
                if (SEO::GetSiteFavicon().empty()) 
                {
                    SEO::SetSiteFavicon(ini.faviconPath);
                }
                if (SEO::GetSiteUrl().empty()) 
                {
                    SEO::SetSiteUrl(ini.siteUrl);
                }
                std::string seoMeta = SEO::GetMetaTags();
                std::string finalHtml = ApplyTemplateFromString(htmlContent, templateString, cssContent, SEO::GetSiteFavicon(), seoMeta);
                finalHtml = AdjustLinks(finalHtml);
                std::ofstream outFile(outputPath);
                if (!outFile)
                {
                    std::cerr << "Error creating output file: " << outputPath << std::endl;
                    continue;
                }
                outFile << finalHtml;
                std::cout << "Generated: " << outputPath << std::endl;
                std::string htmlFilename = relPath.filename().string();
                generatedPages.push_back(relPath.generic_string());

            }
        }
        Meta::GenerateSiteMetaFiles(ini.siteName, cssContent, outputDir, generatedPages);
    }    
}

