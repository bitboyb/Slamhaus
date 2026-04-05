#include "html.hpp"
#include <filesystem>
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

auto GetRelativeRoot(const std::filesystem::path &path) -> std::string {
  auto parent = path.parent_path();
  if (parent.empty()) {
    return "";
  }

  std::string prefix;
  for (const auto &part : parent) {
    if (!part.empty() && part.string() != ".") {
      prefix += "../";
    }
  }
  return prefix;
}

auto PrefixPath(const std::string &path, const std::string &prefix)
    -> std::string {
  if (path.empty()) {
    return path;
  }

  if (path[0] == '/' || path[0] == '#' || path.rfind("http://", 0) == 0 ||
      path.rfind("https://", 0) == 0 || path.rfind("//", 0) == 0 ||
      path.rfind("mailto:", 0) == 0 || path.rfind("tel:", 0) == 0 ||
      path.rfind("data:", 0) == 0) {
    return path;
  }

  return prefix + path;
}
} // namespace HTML
