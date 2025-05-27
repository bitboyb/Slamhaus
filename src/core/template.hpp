#pragma once

#ifndef TEMPLATE_HPP
#define TEMPLATE_HPP

#include <string>
#include "../parser/seo.hpp"
#include "config.hpp"

namespace Template
{
    /// Assembles the base HTML template by parsing header.md, nav.md, and footer.md.
    /// Injects placeholders for title, CSS, content, favicon, and SEO metadata.
    /// @param ini         Config values (site name, URL, etc.)
    /// @param contentDir  Path to the `content` folder containing includes
    /// @return            A raw HTML string with {{...}} placeholders
    std::string BuildTemplate(Config::ConfigINI &ini, 
                              const std::string contentDir);

    /// Replaces all template placeholders ({{ css }}, {{ content }}, etc.) with actual content.
    /// Injects SEO data and inline CSS into the final HTML document.
    /// @param content         The parsed HTML body content
    /// @param templateString  The base HTML template with placeholders
    /// @param cssContent      The inline CSS to inject into <style>
    /// @param seo             Metadata to populate <meta> and <title> tags
    /// @return                A complete HTML string ready to write to file
    std::string ApplyTemplateFromString(const std::string &content, 
                                        const std::string &templateString, 
                                        const std::string &cssContent,
                                        SEO::SEOData &seo);
}

#endif // TEMPLATE_HPP
