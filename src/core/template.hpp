#pragma once

#ifndef TEMPLATE_HPP
#define TEMPLATE_HPP

#include "../parser/seo.hpp"
#include "config.hpp"
#include <string>

namespace Template {
/// Assembles the base HTML template by parsing header.smd, nav.smd, and
/// footer.smd. Injects placeholders for title, linked stylesheets, content,
/// favicon, and SEO metadata.
///
/// @param ini Global site configuration (for title, favicon)
/// @param contentDir Path to content directory (to find partials)
/// @return A raw HTML string with {{CONTENT}}, {{TITLE}}, etc. placeholders.
auto BuildTemplate(const Config::ConfigINI &ini, const std::string &contentDir)
    -> std::string;

/// Injects dynamic content into the base template.
/// Replaces keywords like {{CONTENT}}, {{TITLE}} with actual page data.
///
/// @param content The body HTML of the specific page
/// @param templateString The base template from `BuildTemplate`
/// @param stylesheetTags `<link rel="stylesheet">` tags for the page
/// @param faviconHref The page-relative or absolute favicon path
/// @param seo Specific SEO metadata for this page
/// @return Fully constructed HTML page ready for writing.
auto ApplyTemplateFromString(const std::string &content,
                             const std::string &templateString,
                             const std::string &stylesheetTags,
                             const std::string &faviconHref, SEO::SEOData &seo)
    -> std::string;
} // namespace Template

#endif // TEMPLATE_HPP
