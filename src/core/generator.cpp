#include "generator.hpp"
#include "parser.hpp"
#include "asset.hpp"
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
    
    std::string BuildTemplate()
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
    
        std::string defineMarkdown = readFile("content/include/define.md");
        std::string headerMarkdown = readFile("content/include/header.md");
        std::string navMarkdown = readFile("content/include/nav.md");
        std::string footerMarkdown = readFile("content/include/footer.md");
    
        
        std::string siteName = Parser::ParseMarkdown(defineMarkdown);
        std::string headerContent = Parser::ParseMarkdown(headerMarkdown);
        std::string navContent = Parser::ParseMarkdown(navMarkdown);
        std::string footerContent = Parser::ParseMarkdown(footerMarkdown);
    
        std::string tmpl =
            "<!DOCTYPE html>\n"
            "<html>\n"
            "<head>\n"
            "    <meta charset=\"UTF-8\">\n"
            "    <title>" + siteName + "</title>\n"
            "    <style>\n"
            "    {{ css }}\n"
            "    </style>\n"
            "</head>\n"
            "<body>\n"
            "    <header>\n"
            "       " + headerContent + "\n"
            "    </header>\n"
            "    <nav>\n"
            "       " + navContent + "\n"
            "    </nav>\n"
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
                                        const std::string &cssContent)
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
        return tmplStr;
    }
    
    void GenerateSite(const std::string &contentDir, 
                      const std::string &outputDir, 
                      const std::string &cssPath)
    {
        std::filesystem::create_directories(outputDir);
        std::string cssContent = Parser::LoadCSS(cssPath);
    
        std::string templateString = BuildTemplate();
    
        Asset::CopyAssets("content/assets/", outputDir + "/assets");
    
        for (const auto &entry : std::filesystem::recursive_directory_iterator(contentDir))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".md")
            {
                std::string filename = entry.path().filename().string();
                if (filename == "define.md" || filename == "header.md" ||
                    filename == "nav.md"    || filename == "footer.md")
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
                std::string finalHtml = ApplyTemplateFromString(htmlContent, templateString, cssContent);
                finalHtml = AdjustLinks(finalHtml);
    
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
}

