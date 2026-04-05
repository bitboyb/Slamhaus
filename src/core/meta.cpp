#include "meta.hpp"
#include <fstream>
namespace Meta {
auto GenerateSiteMetaFiles(const std::string &siteURL,
                           const std::string &stylesheetTags,
                           const std::string &outputDir,
                           const std::vector<std::string> &pages) -> void {
  std::ofstream robots(outputDir + "/robots.txt");
  if (robots) {
    robots << "User-agent: *\nDisallow:\n\nSitemap: " << siteURL
           << "/sitemap.xml\n";
  }

  std::ofstream sitemap(outputDir + "/sitemap.xml");
  if (sitemap) {
    sitemap << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    sitemap
        << "<urlset xmlns=\"http://www.sitemaps.org/schemas/sitemap/0.9\">\n";
    for (const auto &page : pages) {
      sitemap << "  <url>\n";
      sitemap << "    <loc>" << siteURL << "/" << page << "</loc>\n";
      sitemap << "  </url>\n";
    }
    sitemap << "</urlset>\n";
  }

  std::ofstream notFound(outputDir + "/404.html");
  if (notFound) {
    notFound << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n"
             << "  <meta charset=\"UTF-8\">\n"
             << "  <title>404 - Page Not Found</title>\n"
             << stylesheetTags
             << "</head>\n<body>\n"
             << "  <main style=\"text-align: center; padding: 50px;\">\n"
             << "    <h1>404</h1>\n"
             << "    <p>Page not found.</p>\n"
             << "    <a href=\"/\">Go Home</a>\n"
             << "  </main>\n</body>\n</html>";
  }
}
} // namespace Meta
