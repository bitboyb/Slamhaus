// File: src/main.cpp
#include <iostream>
#include <fstream>
#include <filesystem>
#include "parser.hpp"

int main() 
{
    std::cout << "Generating site..." << std::endl;

    std::filesystem::create_directories("output");

    std::ifstream mdFile("content/index.md");
    if (!mdFile) 
    {
        std::cerr << "Error opening Markdown file." << std::endl;
        return 1;
    }

    std::string markdown((std::istreambuf_iterator<char>(mdFile)),
                          std::istreambuf_iterator<char>());
    
    std::string htmlContent = ParseMarkdown(markdown);
    std::string cssContent = LoadCSS("assets/css/style.css");
    std::string finalHtml = ApplyTemplate(htmlContent, "templates/base.html", cssContent);

    std::ofstream outFile("output/index.html");
    if (!outFile) 
    {
        std::cerr << "Error creating output file." << std::endl;
        return 1;
    }

    outFile << finalHtml;
    outFile.close();

    std::cout << "Site generated successfully!" << std::endl;
    return 0;
}
