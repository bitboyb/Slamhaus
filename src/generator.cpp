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

std::string BuildTemplate()
{
    auto readFile = [](const std::string &path) -> std::string {
        std::ifstream in(path);
        if (!in) return "";
        return std::string((std::istreambuf_iterator<char>(in)),
                           std::istreambuf_iterator<char>());
    };

    std::string defineMarkdown = readFile("content/include/define.md");
    std::string headerMarkdown = readFile("content/include/header.md");
    std::string navMarkdown = readFile("content/include/nav.md");
    std::string footerMarkdown = readFile("content/include/footer.md");

    
    std::string siteName = ParseMarkdown(defineMarkdown);
    std::string headerContent = ParseMarkdown(headerMarkdown);
    std::string navContent = ParseMarkdown(navMarkdown);
    std::string footerContent = ParseMarkdown(footerMarkdown);

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
    fs::create_directories(outputDir);
    std::string cssContent = LoadCSS(cssPath);

    std::string templateString = BuildTemplate();

    CopyAssets("assets/images", outputDir + "/assets/images");

    for (const auto &entry : fs::recursive_directory_iterator(contentDir))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".md")
        {
            // Skip the partial files.
            std::string filename = entry.path().filename().string();
            if (filename == "define.md" || filename == "header.md" ||
                filename == "nav.md"    || filename == "footer.md")
            {
                std::cout << "Skipping partial file: " << entry.path() << std::endl;
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
