#include "generator.hpp"
#include "parser.hpp"
#include "asset.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>

namespace fs = std::filesystem;

std::string AdjustLinks(const std::string &html)
{
    std::regex mdLinkRegex(R"(href="([^"]+)\.md")");
    return std::regex_replace(html, mdLinkRegex, "href=\"$1.html\"");
}

std::string LoadNavigation(const std::string &navPath)
{
    std::ifstream navFile(navPath);
    if (!navFile) 
    {
        std::cerr << "Navigation file not found: " << navPath << std::endl;
        return "";
    }
    std::string navMarkdown((std::istreambuf_iterator<char>(navFile)),
                            std::istreambuf_iterator<char>());
    return ParseMarkdown(navMarkdown);
}

void GenerateSite(const std::string &contentDir, const std::string &outputDir, const std::string &templatePath, const std::string &cssPath)
{
    fs::create_directories(outputDir);
    std::string cssContent = LoadCSS(cssPath);
    std::string navContent = AdjustLinks(LoadNavigation("content/nav.md"));
    CopyAssets("assets/images", outputDir + "/assets/images");

    for (const auto &entry : fs::recursive_directory_iterator(contentDir))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".md")
        {
            if (entry.path().filename() == "nav.md")
            {
                std::cout << "Skipping navigation file: " << entry.path() << std::endl;
                continue;
            }

            fs::path relPath = fs::relative(entry.path(), contentDir);
            relPath.replace_extension(".html");
            fs::path outputPath = fs::path(outputDir) / relPath;
            fs::create_directories(outputPath.parent_path());

            std::ifstream mdFile(entry.path());
            if (!mdFile)
            {
                std::cerr << "Error opening Markdown file: " << entry.path() << std::endl;
                continue;
            }
            std::string markdown((std::istreambuf_iterator<char>(mdFile)),
                                 std::istreambuf_iterator<char>());

            std::string htmlContent = ParseMarkdown(markdown);
            std::string finalHtml = ApplyTemplate(htmlContent, templatePath, cssContent);
            finalHtml = AdjustLinks(finalHtml);

            size_t navPos = finalHtml.find("{{ nav }}");
            if (navPos != std::string::npos)
            {
                finalHtml.replace(navPos, 9, navContent);
            }

            std::ofstream outFile(outputPath);
            if (!outFile)
            {
                std::cerr << "Error creating output file: " << outputPath << std::endl;
                continue;
            }
            outFile << finalHtml;
            std::cout << "Generated: " << outputPath << std::endl;
        }
    }
}
