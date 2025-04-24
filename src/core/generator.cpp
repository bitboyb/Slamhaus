#include "generator.hpp"
#include "../interpreter/parser.hpp"
#include "../interpreter/seo.hpp"
#include "asset.hpp"
#include "config.hpp"
#include "meta.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include "template.hpp"
#include "../util/html.hpp"
#include "css.hpp"

namespace Generator 
{    
    void GenerateSite(const std::string &contentDir, 
                      const std::string &outputDir, 
                      const std::string &cssPath)
    {
        std::filesystem::create_directories(outputDir);
        std::string cssContent = CSS::LoadCSS(cssPath);
        Config::ConfigINI ini = Config::GetConfig();
        std::vector<std::string> generatedPages;
        std::string templateString = Template::BuildTemplate(ini);
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
                
                SEO::SEOData seo = SEO::GetDefault(ini);
                std::string htmlContent = Parser::ParseMarkdown(markdown, &seo);

                std::string finalHtml = Template::ApplyTemplateFromString(htmlContent, templateString, cssContent, seo);
                finalHtml = HTML::AdjustLinks(finalHtml);
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

