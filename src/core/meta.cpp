#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>

namespace Meta
{
    std::string GetCurrentDate()
    {
        std::time_t t = std::time(nullptr);
        std::tm *now = std::localtime(&t);
        char buf[11];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d", now);
        return std::string(buf);
    }

    std::vector<std::string> GetMarkdownFiles()
    {
        std::vector<std::string> files;
        for (const auto &entry : std::filesystem::directory_iterator("."))
        {
            if (entry.path().extension() == ".md")
            {
                files.push_back(entry.path().stem().string());
            }
        }
        std::sort(files.begin(), files.end());
        return files;
    }

    void GenerateRobotsTxt(const std::string &siteURL, 
                           const std::string &outputDir)
    {
        std::filesystem::create_directories(outputDir);
        std::ofstream out(std::filesystem::path(outputDir) / "robots.txt");
        out << "User-agent: *\n";
        out << "Disallow:\n";
        out << "Sitemap: " << siteURL << "/sitemap.xml\n";
        std::cout << "Generated: " << outputDir << "/robots.txt\n";
    }

    void GenerateSitemap(const std::vector<std::string> &pages,
                         const std::string &siteURL,
                         const std::string &outputDir)
    {
        std::filesystem::create_directories(outputDir);
        std::ofstream out(std::filesystem::path(outputDir) / "sitemap.xml");
        std::string date = GetCurrentDate();
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        out << "<urlset xmlns=\"http://www.sitemaps.org/schemas/sitemap/0.9\">\n";

        for (const auto &page : pages)
        {
            std::string pageUrl = siteURL + "/" + page;
            if (pageUrl.find(".html") == std::string::npos)
            {
                pageUrl += ".html";
            }
            out << "  <url>\n";
            out << "    <loc>" << pageUrl << "</loc>\n";
            out << "    <lastmod>" << date << "</lastmod>\n";
            out << "    <changefreq>weekly</changefreq>\n";
            out << "    <priority>0.8</priority>\n";
            out << "  </url>\n";
        }
        out << "</urlset>\n";
        std::cout << "Generated: " << outputDir << "/sitemap.xml\n";
    }


    void Generate404Page(const std::string &cssContent, 
                         const std::string &outputDir)
    {
        std::filesystem::create_directories(outputDir);
        std::ofstream out(std::filesystem::path(outputDir) / "404.html");
        out << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n";
        out << "  <meta charset=\"UTF-8\">\n";
        out << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
        out << "  <title>Page Not Found</title>\n";
        if (!cssContent.empty())
        {
            out << "  <style>\n" << cssContent << "\n  </style>\n";
        }
        out << "</head>\n<body>\n";
        out << "  <h1>404 - Page Not Found</h1>\n";
        out << "  <p>Sorry, the page you're looking for doesn’t exist.</p>\n";
        out << "  <a href=\"index.html\">Return to homepage</a>\n";
        out << "</body>\n</html>\n";
        std::cout << "Generated: " << outputDir << "/404.html\n";
    }

    void GenerateSiteMetaFiles(const std::string &siteURL,
                               const std::string &cssContent,
                               const std::string &outputDir,
                               const std::vector<std::string> &pages)
    {
        GenerateRobotsTxt(siteURL, outputDir);
        GenerateSitemap(pages, siteURL, outputDir);
        Generate404Page(cssContent, outputDir);
    }
}
