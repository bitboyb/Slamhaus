#pragma once

#ifndef TEMPLATE_HPP
#define TEMPLATE_HPP

#include <string>

namespace Template
{
    std::string ReadTemplateFile(const std::string &templatePath);
    
    void ReplacePlaceholder(std::string &tmplStr, 
                            const std::string &placeholder, 
                            const std::string &value);

    std::string ApplyTemplate(const std::string &content,
                              const std::string &templatePath,
                              const std::string &cssContent);
}

#endif // TEMPLATE_HPP
