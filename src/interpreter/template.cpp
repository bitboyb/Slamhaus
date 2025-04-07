#include <sstream>
#include <fstream>
#include <iostream>
#include <cctype>

namespace Template
{
    std::string ReadTemplateFile(const std::string &templatePath)
    {
        std::ifstream tmplFile(templatePath);
        if (!tmplFile)
        {
            std::cerr << "Error opening template file: " << templatePath << std::endl;
            return "";
        }
        std::ostringstream tmplStream;
        tmplStream << tmplFile.rdbuf();
        return tmplStream.str();
    }

    void ReplacePlaceholder(std::string &tmplStr, 
                            const std::string &placeholder, 
                            const std::string &value)
    {
        size_t pos = tmplStr.find(placeholder);
        if (pos != std::string::npos)
        {
            tmplStr.replace(pos, placeholder.length(), value);
        }
        else if (placeholder == "{{ content }}")
        {
            std::cerr << "Warning: Content placeholder not found in template. Appending content." << std::endl;
            tmplStr += value;
        }
        else
        {
            std::cerr << "Warning: " << placeholder << " placeholder not found in template." << std::endl;
        }
    }

    std::string ApplyTemplate(const std::string &content,
                              const std::string &templatePath,
                              const std::string &cssContent)
    {
        std::string tmplStr = ReadTemplateFile(templatePath);
        if (tmplStr.empty())
        {
            return content;
        }
        ReplacePlaceholder(tmplStr, "{{ css }}", cssContent);
        ReplacePlaceholder(tmplStr, "{{ content }}", content);
        return tmplStr;
    }
}