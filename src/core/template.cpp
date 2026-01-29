#include "template.hpp"
#include "../parser/nav.hpp"
#include "../parser/parser.hpp"
#include <fstream>

#include <sstream>
#include <vector>

namespace Template {
auto BuildTemplate(const Config::ConfigINI &ini, const std::string &contentDir)
    -> std::string {
  std::ostringstream html;

  auto readFile = [](const std::string &basePath) -> std::string {
    const std::vector<std::string> extensions = {".smd", ".slam"};
    for (const auto &ext : extensions) {
      std::ifstream in(basePath + ext);
      if (in) {
        return std::string((std::istreambuf_iterator<char>(in)),
                           std::istreambuf_iterator<char>());
      }
    }
    return "";
  };

  std::string rawHeader = readFile(contentDir + "/include/header");
  std::string rawNav = readFile(contentDir + "/include/nav");
  std::string rawFooter = readFile(contentDir + "/include/footer");

  std::string header = Parser::ParseMarkdown(rawHeader);
  std::string nav = Nav::ParseNavigation(rawNav);
  std::string footer = Parser::ParseMarkdown(rawFooter);

  html << "<!DOCTYPE html>\n"
       << "<html lang=\"en\">\n"
       << "<head>\n"
       << "  <meta charset=\"UTF-8\">\n"
       << "  <meta name=\"viewport\" content=\"width=device-width, "
          "initial-scale=1.0\">\n"
       << "  <title>{{TITLE}}</title>\n"
       << "  <style>{{CSS}}</style>\n"
       << "  <link rel=\"icon\" href=\"" << ini.faviconPath
       << "\" type=\"image/x-icon\">\n"
       << "  {{SEO_META}}\n"
       << "  " << header << "\n"
       << "</head>\n"
       << "<body>\n"
       << "  <header>" << ini.siteName << "</header>\n"
       << "  <nav>" << nav << "</nav>\n"
       << "  <main>\n"
       << "    {{CONTENT}}\n"
       << "  </main>\n"
       << "  <footer>" << footer << "</footer>\n"
       << "</body>\n"
       << "</html>";

  return html.str();
}

auto ApplyTemplateFromString(const std::string &content,
                             const std::string &templateString,
                             const std::string &cssContent, SEO::SEOData &seo)
    -> std::string {
  std::string out = templateString;
  std::string seoMeta = SEO::GetMetaTags(seo);

  auto replaceAll = [&](std::string &s, const std::string &from,
                        const std::string &to) {
    size_t pos = 0;
    while ((pos = s.find(from, pos)) != std::string::npos) {
      s.replace(pos, from.length(), to);
      pos += to.length();
    }
  };

  std::string title = seo.siteTitle.empty() ? "Slamhaus Site" : seo.siteTitle;
  replaceAll(out, "{{TITLE}}", title);
  replaceAll(out, "{{CSS}}", cssContent);
  replaceAll(out, "{{CONTENT}}", content);
  replaceAll(out, "{{SEO_META}}", seoMeta);

  return out;
}
} // namespace Template
