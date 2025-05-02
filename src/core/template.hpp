#pragma once

#ifndef TEMPLATE_HPP
#define TEMPLATE_HPP

#include <string>
#include "../parser/seo.hpp"
#include "config.hpp"

namespace Template
{
    std::string BuildTemplate(Config::ConfigINI &ini, 
                              const std::string contentDir);

    std::string ApplyTemplateFromString(const std::string &content, 
                                        const std::string &templateString, 
                                        const std::string &cssContent,
                                        SEO::SEOData &seo);
}

#endif // TEMPLATE_HPP
