#include "css.hpp"
#include "../util/html.hpp"
#include <algorithm>
#include <filesystem>
#include <sstream>

namespace CSS {
auto CollectStylesheets(const std::filesystem::path &directoryPath)
    -> std::vector<std::string> {
  namespace fs = std::filesystem;
  std::vector<std::string> stylesheets;

  if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
    return stylesheets;
  }

  for (const auto &entry : fs::recursive_directory_iterator(
           directoryPath, fs::directory_options::skip_permission_denied)) {
    if (entry.is_regular_file() && entry.path().extension() == ".css") {
      stylesheets.push_back(
          fs::relative(entry.path(), directoryPath).generic_string());
    }
  }

  std::sort(stylesheets.begin(), stylesheets.end());
  return stylesheets;
}

auto BuildStylesheetTags(const std::vector<std::string> &stylesheets,
                         const std::string &rootPrefix) -> std::string {
  std::ostringstream html;
  for (const auto &stylesheet : stylesheets) {
    html << "  <link rel=\"stylesheet\" href=\""
         << HTML::PrefixPath("assets/theme/" + stylesheet, rootPrefix)
         << "\">\n";
  }
  return html.str();
}
} // namespace CSS
