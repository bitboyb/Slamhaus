#include "html.hpp"
#include <regex>
#include <string>

namespace HTML {
auto AdjustLinks(const std::string &html) -> std::string {
  std::regex extRegex(R"(href="([^"]+)\.(smd|slam))");
  return std::regex_replace(html, extRegex, "href=\"$1.html");
}

auto AdjustPath(const std::string &path) -> std::string {
  size_t hashPos = path.find('#');
  std::string base =
      hashPos != std::string::npos ? path.substr(0, hashPos) : path;
  std::string fragment =
      hashPos != std::string::npos ? path.substr(hashPos) : "";

  std::regex extRegex(R"((\.smd|\.slam)$)");
  base = std::regex_replace(base, extRegex, ".html");

  return base + fragment;
}
} // namespace HTML
