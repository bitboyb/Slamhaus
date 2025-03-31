#include "parser.hpp"
#include <sstream>
#include <fstream>
#include <iostream>

std::string ParseMarkdown(const std::string& markdown) 
{
    std::istringstream iss(markdown);
    std::string line;
    std::ostringstream html;
    
    while (std::getline(iss, line)) 
    {
        if (line.find("# ") == 0) 
        {
            html << "<h1>" << line.substr(2) << "</h1>\n";
        } 
        else if (line.find("## ") == 0) 
        {
            html << "<h2>" << line.substr(3) << "</h2>\n";
        } 
        else if (line.find("### ") == 0) 
        {
            html << "<h3>" << line.substr(4) << "</h3>\n";
        } 
        else 
        {
            if (!line.empty())
            {
                html << "<p>" << line << "</p>\n";
            }
        }
    }
    
    return html.str();
}

std::string LoadCSS(const std::string &cssPath) {
    std::ifstream cssFile(cssPath);
    if (!cssFile) {
        std::cerr << "Error opening CSS file: " << cssPath << std::endl;
        return "";
    }
    std::ostringstream cssStream;
    cssStream << cssFile.rdbuf();
    return cssStream.str();
}

std::string ApplyTemplate(const std::string &content, const std::string &templatePath, const std::string &cssContent)
{
    std::ifstream tmplFile(templatePath);
    if (!tmplFile) 
    {
        std::cerr << "Error opening template file: " << templatePath << std::endl;
        return content;
    }
    std::ostringstream tmplStream;
    tmplStream << tmplFile.rdbuf();
    std::string tmplStr = tmplStream.str();

    std::string cssPlaceholder = "{{ css }}";
    size_t pos = tmplStr.find(cssPlaceholder);
    if (pos != std::string::npos) 
    {
        tmplStr.replace(pos, cssPlaceholder.length(), cssContent);
    }

    std::string contentPlaceholder = "{{ content }}";
    pos = tmplStr.find(contentPlaceholder);

    if (pos != std::string::npos) 
    {
        tmplStr.replace(pos, contentPlaceholder.length(), content);
    } 
    else 
    {
        tmplStr += content;
    }

    return tmplStr;
}
