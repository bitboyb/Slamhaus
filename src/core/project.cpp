#include "core/project.hpp"

#include "version.hpp"
#include "wasm/wasm.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <fstream>
#include <regex>
#include <set>
#include <sstream>
#include <system_error>

#include <yaml-cpp/yaml.h>

namespace Project {
namespace {

struct Config {
  std::string name = "My Site";
  std::string url;
  std::string basePath = "/";
  std::string language = "en";
  std::string description;
  std::filesystem::path output = "dist";
  std::vector<std::string> styles;
  std::string header;
  std::string navigation;
  std::string footer;
  bool prettyUrls = true;
  bool unsafeHtml = false;
  bool pwa = false;
  bool feed = false;
};

struct ParsedPage {
  std::filesystem::path source;
  std::filesystem::path route;
  std::string module;
  Language::Document document;
};

auto Read(const std::filesystem::path &path, std::string &content) -> bool {
  std::ifstream input(path, std::ios::binary);
  if (!input)
    return false;
  content.assign(std::istreambuf_iterator<char>(input), {});
  return input.good() || input.eof();
}

auto Write(const std::filesystem::path &path, const std::string &content) -> bool {
  std::filesystem::create_directories(path.parent_path());
  std::ofstream output(path, std::ios::binary);
  output << content;
  return output.good();
}

auto NormalizeBasePath(std::string value) -> std::string {
  if (value.empty() || value.front() != '/')
    value.insert(value.begin(), '/');
  if (value.back() != '/')
    value.push_back('/');
  return value;
}

auto Lower(std::string value) -> std::string {
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char character) {
                   return static_cast<char>(std::tolower(character));
                 });
  return value;
}

auto HasEncodedTraversal(const std::string &value) -> bool {
  const auto lower = Lower(value);
  return lower.find("%2e") != std::string::npos ||
         lower.find("%2f") != std::string::npos ||
         lower.find("%5c") != std::string::npos;
}

auto HasUnknownScheme(const std::string &value) -> bool {
  const auto colon = value.find(':');
  const auto slash = value.find('/');
  return colon != std::string::npos && (slash == std::string::npos || colon < slash);
}

auto Diagnostic(const std::filesystem::path &path, std::string code,
                std::string message, std::string help = {}, Language::Range range = {})
    -> Language::Diagnostic {
  return {path,
          Language::Severity::Error,
          std::move(code),
          std::move(message),
          std::move(help),
          range};
}

auto LoadConfig(const std::filesystem::path &path, Config &config)
    -> std::vector<Language::Diagnostic> {
  std::vector<Language::Diagnostic> diagnostics;
  if (!std::filesystem::is_regular_file(path)) {
    diagnostics.push_back(Diagnostic(path, "SH4013", "slamhaus.yaml is missing."));
    return diagnostics;
  }
  try {
    const auto yaml = YAML::LoadFile(path.string());
    if (!yaml || !yaml.IsMap())
      throw YAML::ParserException(YAML::Mark{}, "configuration must be a mapping");
    const std::set<std::string> rootKeys = {"slamhaus", "site",   "build",
                                            "theme",    "layout", "pwa", "feed"};
    for (const auto &entry : yaml) {
      const auto key = entry.first.as<std::string>();
      if (rootKeys.count(key) == 0U)
        diagnostics.push_back(Diagnostic(
            path, "SH4015", "Unknown configuration section " + key + ".", {},
            {{static_cast<std::size_t>(entry.first.Mark().line),
              static_cast<std::size_t>(entry.first.Mark().column)},
             {static_cast<std::size_t>(entry.first.Mark().line),
              static_cast<std::size_t>(entry.first.Mark().column) + key.size()}}));
    }
    const auto readMap = [&](const char *name, const std::set<std::string> &known) {
      const auto map = yaml[name];
      if (!map)
        return;
      if (!map.IsMap())
        throw YAML::ParserException(map.Mark(),
                                    std::string(name) + " must be a mapping");
      for (const auto &entry : map) {
        const auto key = entry.first.as<std::string>();
        if (known.count(key) == 0U)
          diagnostics.push_back(Diagnostic(
              path, "SH4015", "Unknown " + std::string(name) + " setting " + key + ".",
              {},
              {{static_cast<std::size_t>(entry.first.Mark().line),
                static_cast<std::size_t>(entry.first.Mark().column)},
               {static_cast<std::size_t>(entry.first.Mark().line),
                static_cast<std::size_t>(entry.first.Mark().column) + key.size()}}));
      }
    };
    readMap("slamhaus", {"version"});
    readMap("site", {"name", "url", "base_path", "language", "description"});
    readMap("build", {"output", "pretty_urls", "unsafe_html"});
    readMap("theme", {"builtin", "styles"});
    readMap("layout", {"header", "navigation", "footer"});
    readMap("pwa", {"enabled"});
    readMap("feed", {"enabled"});
    if (yaml["site"]) {
      const auto site = yaml["site"];
      if (site["name"])
        config.name = site["name"].as<std::string>();
      if (site["url"])
        config.url = site["url"].as<std::string>();
      if (site["base_path"])
        config.basePath = site["base_path"].as<std::string>();
      if (site["language"])
        config.language = site["language"].as<std::string>();
      if (site["description"])
        config.description = site["description"].as<std::string>();
    }
    if (yaml["build"]) {
      const auto build = yaml["build"];
      if (build["output"])
        config.output = build["output"].as<std::string>();
      if (build["pretty_urls"])
        config.prettyUrls = build["pretty_urls"].as<bool>();
      if (build["unsafe_html"])
        config.unsafeHtml = build["unsafe_html"].as<bool>();
    }
    if (yaml["theme"] && yaml["theme"]["styles"])
      for (const auto &style : yaml["theme"]["styles"])
        config.styles.push_back(style.as<std::string>());
    if (yaml["theme"] && yaml["theme"]["builtin"] &&
        yaml["theme"]["builtin"].as<std::string>() != "default")
      diagnostics.push_back(Diagnostic(
          path, "SH4028", "Unknown built-in theme. Only default is available."));
    if (yaml["layout"]) {
      const auto layout = yaml["layout"];
      if (layout["header"])
        config.header = layout["header"].as<std::string>();
      if (layout["navigation"])
        config.navigation = layout["navigation"].as<std::string>();
      if (layout["footer"])
        config.footer = layout["footer"].as<std::string>();
    }
    if (yaml["pwa"] && yaml["pwa"]["enabled"])
      config.pwa = yaml["pwa"]["enabled"].as<bool>();
    if (yaml["feed"] && yaml["feed"]["enabled"])
      config.feed = yaml["feed"]["enabled"].as<bool>();
  } catch (const YAML::Exception &error) {
    diagnostics.push_back(Diagnostic(
        path, "SH4014", "Invalid project YAML: " + std::string(error.what()), {},
        {{static_cast<std::size_t>(std::max(0, error.mark.line)),
          static_cast<std::size_t>(std::max(0, error.mark.column))},
         {static_cast<std::size_t>(std::max(0, error.mark.line)),
          static_cast<std::size_t>(std::max(0, error.mark.column + 1))}}));
  }
  config.basePath = NormalizeBasePath(config.basePath);
  while (config.url.size() > 1 && config.url.back() == '/')
    config.url.pop_back();
  if (config.basePath.find("..") != std::string::npos ||
      config.basePath.find('\\') != std::string::npos)
    diagnostics.push_back(
        Diagnostic(path, "SH4018", "site.base_path cannot contain traversal."));
  if (config.output.empty() || config.output.is_absolute() ||
      config.output.lexically_normal().empty() ||
      *config.output.lexically_normal().begin() == "..")
    diagnostics.push_back(
        Diagnostic(path, "SH4009",
                   "build.output must be a relative directory inside the project."));
  return diagnostics;
}

auto MetadataDefaults(const Config &config) -> Language::Metadata {
  Language::Metadata metadata;
  metadata.siteTitle = config.name;
  metadata.siteDescription = config.description;
  metadata.siteUrl = config.url;
  return metadata;
}

auto SourceFiles(const std::filesystem::path &root)
    -> std::vector<std::filesystem::path> {
  std::vector<std::filesystem::path> files;
  if (!std::filesystem::is_directory(root))
    return files;
  for (const auto &entry : std::filesystem::recursive_directory_iterator(root))
    if (entry.is_regular_file() &&
        (entry.path().extension() == ".md" || entry.path().extension() == ".slam"))
      files.push_back(entry.path());
  std::sort(files.begin(), files.end());
  return files;
}

auto PageRoute(const std::filesystem::path &source, const std::filesystem::path &pages,
               const Language::Metadata &metadata, bool pretty,
               std::vector<Language::Diagnostic> &diagnostics)
    -> std::filesystem::path {
  if (!metadata.permalink.empty()) {
    const auto &value = metadata.permalink;
    if (value.front() != '/' || value.find("..") != std::string::npos ||
        value.find_first_of("?#\\") != std::string::npos) {
      diagnostics.push_back(Diagnostic(source, "SH4021",
                                       "permalink must be an absolute site path "
                                       "without traversal, query, or fragment."));
      return {};
    }
    auto route = std::filesystem::path(value.substr(1));
    if (value.back() == '/' || route.extension().empty())
      route /= "index.html";
    return route;
  }
  auto relative = std::filesystem::relative(source, pages);
  relative.replace_extension();
  if (relative.filename() == "index") {
    relative.replace_filename("index.html");
  } else if (pretty) {
    relative /= "index.html";
  } else {
    relative.replace_extension(".html");
  }
  return relative;
}

auto PublicRoute(const std::filesystem::path &route, const std::string &basePath)
    -> std::string {
  auto value = route.generic_string();
  if (value == "index.html")
    value.clear();
  else if (value.size() >= 10 && value.substr(value.size() - 10) == "index.html")
    value.erase(value.size() - 10);
  return basePath + value;
}

auto ModuleName(const std::filesystem::path &source, const std::filesystem::path &pages)
    -> std::string {
  auto relative = std::filesystem::relative(source, pages);
  relative.replace_extension();
  std::string module;
  for (char character : relative.generic_string())
    module.push_back(
        std::isalnum(static_cast<unsigned char>(character)) != 0
            ? static_cast<char>(std::tolower(static_cast<unsigned char>(character)))
            : '-');
  return module.empty() ? "index" : module;
}

auto ResolvePageUrl(
    const std::string &authored, const std::filesystem::path &source,
    const std::filesystem::path &root,
    const std::map<std::filesystem::path, std::filesystem::path> &routes,
    const Config &config, std::vector<Language::Diagnostic> &diagnostics)
    -> std::string {
  const auto lower = Lower(authored);
  if (authored.empty() || authored.front() == '#' ||
      authored.compare(0, 2, "//") == 0 || lower.compare(0, 7, "http://") == 0 ||
      lower.compare(0, 8, "https://") == 0 || lower.compare(0, 5, "data:") == 0 ||
      lower.compare(0, 7, "mailto:") == 0 || lower.compare(0, 4, "tel:") == 0)
    return authored;
  if (HasUnknownScheme(authored) || HasEncodedTraversal(authored)) {
    diagnostics.push_back(
        Diagnostic(source, "SH4010", "URL uses an unsafe scheme or encoded path."));
    return authored;
  }
  const auto suffix = authored.find_first_of("?#");
  const auto rawPath =
      suffix == std::string::npos ? authored : authored.substr(0, suffix);
  const auto tail = suffix == std::string::npos ? "" : authored.substr(suffix);
  if (rawPath.empty())
    return authored;
  if (std::regex_match(
          rawPath,
          std::regex(R"(^[A-Za-z0-9.-]+\.(com|org|net|io|dev|co|uk|me|tv|ai)$)"))) {
    diagnostics.push_back(Diagnostic(
        source, "SH4011",
        "URL looks like an external domain but has no scheme: " + rawPath,
        "Use https:// or http:// explicitly."));
    return authored;
  }
  if (rawPath.front() == '/')
    return config.basePath + rawPath.substr(1) + tail;
  std::filesystem::path target;
  if (rawPath.compare(0, 7, "assets/") == 0)
    target = root / rawPath;
  else
    target = source.parent_path() / rawPath;
  target = std::filesystem::absolute(target).lexically_normal();
  const auto normalizedRoot = std::filesystem::absolute(root).lexically_normal();
  const auto relative = target.lexically_relative(normalizedRoot);
  if (relative.empty() || *relative.begin() == "..") {
    diagnostics.push_back(Diagnostic(source, "SH4010", "URL escapes the project."));
    return authored;
  }
  const auto route = routes.find(target);
  if (route != routes.end())
    return PublicRoute(route->second, config.basePath) + tail;
  if (!std::filesystem::exists(target))
    diagnostics.push_back(
        Diagnostic(source, "SH4011", "Local URL target does not exist: " + authored));
  if (relative.extension() == ".md" || relative.extension() == ".slam") {
    diagnostics.push_back(
        Diagnostic(source, "SH4011",
                   "Page link is not part of the generated routes: " + authored));
    return authored;
  }
  return config.basePath + relative.generic_string() + tail;
}

auto FindPartial(const std::filesystem::path &partials, const std::string &name)
    -> std::filesystem::path {
  const auto requested = std::filesystem::path(name);
  if (requested.is_absolute() || name.find("..") != std::string::npos)
    return {};
  auto candidate = partials / requested;
  if (candidate.extension().empty()) {
    auto slam = candidate;
    slam.replace_extension(".slam");
    if (std::filesystem::is_regular_file(slam))
      return slam;
    candidate.replace_extension(".md");
  }
  return std::filesystem::is_regular_file(candidate) ? candidate
                                                     : std::filesystem::path{};
}

auto ExpandPartials(std::vector<Language::Node> nodes,
                    const std::filesystem::path &partials,
                    const Language::Metadata &metadata,
                    std::vector<Language::Diagnostic> &diagnostics,
                    std::vector<std::filesystem::path> stack = {})
    -> std::vector<Language::Node> {
  std::vector<Language::Node> expanded;
  for (auto &node : nodes) {
    if (node.kind == Language::NodeKind::Component && node.name == "Include") {
      const auto name =
          node.properties.count("name") == 0U ? "" : node.properties.at("name");
      const auto path = FindPartial(partials, name);
      if (path.empty()) {
        diagnostics.push_back(Diagnostic(
            {}, "SH4027", "Partial does not exist: " + name, {}, node.range));
        continue;
      }
      const auto normalized = std::filesystem::absolute(path).lexically_normal();
      if (std::find(stack.begin(), stack.end(), normalized) != stack.end()) {
        diagnostics.push_back(Diagnostic(
            path, "SH4024", "Partial include cycle detected.", {}, node.range));
        continue;
      }
      std::string source;
      Read(path, source);
      auto partial = Language::Parse(source, path, metadata);
      diagnostics.insert(diagnostics.end(), partial.diagnostics.begin(),
                         partial.diagnostics.end());
      auto nested = stack;
      nested.push_back(normalized);
      auto values = ExpandPartials(std::move(partial.nodes), partials, metadata,
                                   diagnostics, std::move(nested));
      expanded.insert(expanded.end(), std::make_move_iterator(values.begin()),
                      std::make_move_iterator(values.end()));
      continue;
    }
    node.children = ExpandPartials(std::move(node.children), partials, metadata,
                                   diagnostics, stack);
    expanded.push_back(std::move(node));
  }
  return expanded;
}

auto RenderPartial(const std::string &name, const std::filesystem::path &partials,
                   const Language::Metadata &metadata,
                   const Language::RenderOptions &options,
                   std::vector<Language::Diagnostic> &diagnostics) -> std::string {
  if (name.empty())
    return {};
  const auto path = FindPartial(partials, name);
  if (path.empty()) {
    diagnostics.push_back(
        Diagnostic(partials, "SH4027", "Layout partial does not exist: " + name));
    return {};
  }
  std::string source;
  Read(path, source);
  // Layout regions are independent documents. Do not inherit page runtime
  // metadata, otherwise every header/nav/footer would emit the page module.
  auto document = Language::Parse(source, path);
  document.nodes = ExpandPartials(std::move(document.nodes), partials, metadata,
                                  diagnostics, {path});
  auto rendered = Language::Render(document, options);
  diagnostics.insert(diagnostics.end(), rendered.diagnostics.begin(),
                     rendered.diagnostics.end());
  return rendered.html;
}

auto BaseCss() -> std::string {
  return R"CSS(*,*::before,*::after{box-sizing:border-box}
html{font-family:system-ui,sans-serif;line-height:1.5;color-scheme:light dark}
body{margin:0}main,body>header,body>nav,body>footer{width:min(72rem,100% - 2rem);margin-inline:auto}
main{padding-block:1.5rem}img,video,canvas{max-width:100%;height:auto}
[data-slam-layout=row],[data-slam-layout=column]{display:flex;gap:1rem}
[data-slam-layout=row]{flex-flow:row wrap}[data-slam-layout=column]{flex-direction:column}
label{display:grid;gap:.25rem}button,input,select,textarea{font:inherit;padding:.5rem}
output{display:block;min-height:1.5em}[data-slamhaus-error]{color:#b42318}
pre{overflow:auto;padding:1rem;background:#171717;color:#f5f5f5;border-radius:4px}
.tok-keyword,.tok-meta{color:#ff7ab2}.tok-string{color:#a8cc8c}.tok-number{color:#d4bfff}.tok-comment{color:#8b949e}
@media(max-width:40rem){[data-slam-layout=row]{flex-direction:column}}
)CSS";
}

auto PageHtml(const Language::Document &document, const Config &config,
              const std::string &body, const std::string &header,
              const std::string &navigation, const std::string &footer,
              const std::string &canonical) -> std::string {
  const auto title =
      document.metadata.pageTitle.empty() ? config.name : document.metadata.pageTitle;
  const auto description = document.metadata.pageDescription.empty()
                               ? config.description
                               : document.metadata.pageDescription;
  std::ostringstream html;
  html << "<!doctype html>\n<html lang=\"" << Language::EscapeHtml(config.language)
       << "\">\n<head>\n<meta charset=\"utf-8\">\n"
          "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">\n"
       << "<title>" << Language::EscapeHtml(title) << "</title>\n";
  if (!description.empty())
    html << "<meta name=\"description\" content=\"" << Language::EscapeHtml(description)
         << "\">\n";
  if (!canonical.empty())
    html << "<link rel=\"canonical\" href=\"" << Language::EscapeHtml(canonical)
         << "\">\n";
  if (document.metadata.noindex)
    html << "<meta name=\"robots\" content=\"noindex\">\n";
  html << "<link rel=\"stylesheet\" href=\"" << config.basePath << "slamhaus.css\">\n";
  for (const auto &style : config.styles)
    html << "<link rel=\"stylesheet\" href=\"" << config.basePath
         << Language::EscapeHtml(style) << "\">\n";
  if (config.pwa)
    html << "<link rel=\"manifest\" href=\"" << config.basePath
         << "manifest.webmanifest\">\n";
  html << "</head>\n<body>\n";
  if (!header.empty())
    html << "<header class=\"slamhaus-header\">\n" << header << "</header>\n";
  if (!navigation.empty())
    html << "<nav class=\"slamhaus-navigation\" aria-label=\"Primary\">\n"
         << navigation << "</nav>\n";
  html << "<main>\n"
       << body << "</main>\n"
       ;
  if (!footer.empty())
    html << "<footer class=\"slamhaus-footer\">\n" << footer << "</footer>\n";
  if (config.pwa)
    html << "<script type=\"module\" src=\"" << config.basePath
         << "slamhaus-pwa.js\"></script>\n";
  html << "</body>\n</html>\n";
  return html.str();
}

auto ProtectedOutput(const std::filesystem::path &root,
                     const std::filesystem::path &output) -> bool {
  const auto normalizedRoot = std::filesystem::absolute(root).lexically_normal();
  const auto normalized = std::filesystem::absolute(output).lexically_normal();
  if (normalized == normalizedRoot || normalized == normalizedRoot.root_path())
    return true;
  for (const auto *name : {"pages", "assets", "partials", "code", ".git"})
    if (normalized == normalizedRoot / name)
      return true;
  return false;
}

auto CopyTree(const std::filesystem::path &from, const std::filesystem::path &to,
              Result &result, const std::vector<std::string> &ignore = {}) -> void {
  if (!std::filesystem::is_directory(from))
    return;
  for (const auto &entry : std::filesystem::recursive_directory_iterator(from)) {
    if (!entry.is_regular_file())
      continue;
    const auto relative = std::filesystem::relative(entry.path(), from).generic_string();
    const auto ignored = std::any_of(ignore.begin(), ignore.end(), [&](const auto &pattern) {
      std::string expression = "^";
      for (const auto character : pattern) {
        if (character == '*')
          expression += ".*";
        else if (character == '?')
          expression += ".";
        else if (std::string(".^$|()[]{}+\\").find(character) != std::string::npos)
          expression += std::string("\\") + character;
        else
          expression += character;
      }
      expression += "$";
      const std::regex matcher(expression);
      return std::regex_match(relative, matcher) ||
             std::regex_match(entry.path().filename().generic_string(), matcher);
    });
    if (ignored)
      continue;
    const auto destination = to / std::filesystem::relative(entry.path(), from);
    std::filesystem::create_directories(destination.parent_path());
    std::error_code error;
    std::filesystem::copy_file(entry.path(), destination,
                               std::filesystem::copy_options::overwrite_existing,
                               error);
    if (error)
      result.diagnostics.push_back(
          Diagnostic(entry.path(), "SH4003", "Could not copy asset."));
    else
      result.outputs.push_back(destination);
  }
}

auto AssetIgnorePatterns(const std::filesystem::path &root) -> std::vector<std::string> {
  std::vector<std::string> patterns;
  std::string source;
  if (!Read(root / ".slamhausignore", source))
    return patterns;
  std::istringstream lines(source);
  std::string line;
  while (std::getline(lines, line)) {
    const auto first = line.find_first_not_of(" \t\r");
    if (first == std::string::npos || line[first] == '#')
      continue;
    const auto last = line.find_last_not_of(" \t\r");
    patterns.push_back(line.substr(first, last - first + 1));
  }
  return patterns;
}

auto GeneratedCpp(const ParsedPage &page) -> std::string {
  std::ostringstream source;
  source << "#include <slam/slam.hpp>\nusing namespace Slam;\n#line "
         << page.document.behavior.range.start.line + 2 << " \""
         << page.source.generic_string() << "\"\n"
         << page.document.behavior.source << "\n#line 1 \"slamhaus-generated\"\n";
  for (const auto &name : page.document.behavior.exports)
    source << "SLAM_BIND(" << name << ");\n";
  if (!page.document.metadata.runtime.application.empty())
    source << "SLAM_IMPLEMENT_APPLICATION("
           << page.document.metadata.runtime.application << ");\n";
  return source.str();
}

auto ValidateBehavior(const ParsedPage &page,
                      std::vector<Language::Diagnostic> &diagnostics) -> void {
  for (const auto &name : page.document.behavior.exports) {
    const std::regex function("(^|[^A-Za-z0-9_])" + name + "\\s*\\(");
    const auto begin =
        std::sregex_iterator(page.document.behavior.source.begin(),
                             page.document.behavior.source.end(), function);
    const auto end = std::sregex_iterator();
    const auto count = static_cast<std::size_t>(std::distance(begin, end));
    if (count == 0U)
      diagnostics.push_back(Diagnostic(page.source, "SH1124",
                                       "UI calls missing C++ function " + name + ".",
                                       {}, page.document.behavior.range));
    else if (count > 1U)
      diagnostics.push_back(Diagnostic(
          page.source, "SH1125",
          "Automatically exported function " + name + " cannot be overloaded.",
          "Move overloads into shared code and bind an unambiguous wrapper.",
          page.document.behavior.range));
  }
  const auto &application = page.document.metadata.runtime.application;
  if (!application.empty() &&
      page.document.behavior.source.find("class " + application) == std::string::npos)
    diagnostics.push_back(
        Diagnostic(page.source, "SH1126",
                   "runtime.application class was not found: " + application, {},
                   page.document.behavior.range));
}

auto GenerateFiles(
    const std::vector<std::pair<std::filesystem::path, std::string>> &files,
    const std::filesystem::path &root, bool force) -> Result {
  Result result;
  if (std::filesystem::exists(root) && !std::filesystem::is_directory(root)) {
    result.diagnostics.push_back(
        Diagnostic(root, "SH4001", "Project path is not a directory."));
    return result;
  }
  for (const auto &[relative, content] : files) {
    const auto destination = root / relative;
    if (std::filesystem::exists(destination) && !force) {
      result.diagnostics.push_back(
          Diagnostic(destination, "SH4002", "Project file already exists.",
                     "Use --force to replace generated files."));
      continue;
    }
    if (!Write(destination, content))
      result.diagnostics.push_back(
          Diagnostic(destination, "SH4003", "Could not write project file."));
    else
      result.outputs.push_back(destination);
  }
  result.success = !Language::HasErrors(result.diagnostics);
  return result;
}

} // namespace

auto BlankProjectFiles() -> std::vector<std::pair<std::filesystem::path, std::string>> {
  return {
      {"slamhaus.yaml", "slamhaus:\n  version: " + std::string(SLAMHAUS_VERSION) +
                           "\nsite:\n  name: My Site\nlayout:\n  footer: footer\n"},
      {"pages/index.slam", "# My Site\n"},
      {"partials/footer.md", "Made with [Slamhaus](https://www.slamhaus.com).\n"},
      {"AGENTS.md",
       "# Slamhaus site\n\nPages are GFM `.md` or enhanced `.slam` files. "
       "Use C++-style `slam-ui` and `cpp wasm` fences for interaction; do not write "
       "JavaScript for supported behavior. Run `slamhaus check --strict` and "
       "`slamhaus build` before publishing. Generated output belongs in `dist/`.\n"},
      {".gitignore", "dist/\n.slamhaus/\n"}};
}

auto ExampleProjectFiles(ExampleProfile profile)
    -> std::vector<std::pair<std::filesystem::path, std::string>> {
  auto files = BlankProjectFiles();
  files[0].second =
      "slamhaus:\n  version: " + std::string(SLAMHAUS_VERSION) +
      "\nsite:\n  name: Slamhaus Examples\n  description: Generated Slamhaus project\n"
      "theme:\n  styles: [assets/site.css]\nlayout:\n  navigation: nav\n";
  files[1].second =
      "---\ntitle: Examples\ntoc: true\n---\n# Slamhaus Examples\n\n"
      "This project was generated by the CLI.\n\n## Highlighting\n\n"
      "```cpp\nauto Score(int32 Points) -> int32 { return Points * 10; }\n```\n";
  files.push_back(
      {"pages/about.md",
       "---\ntitle: About\n---\n# Portable Markdown\n\n[Home](index.slam)\n"});
  files.push_back({"partials/nav.md", "[Home](index.slam) · [About](about.md)\n"});
  files.push_back({"assets/site.css", "h1{color:#d13f69}\n"});
  if (profile == ExampleProfile::Complete) {
    files[0].second += "pwa:\n  enabled: true\n";
    const auto toc = files[1].second.find("toc: true\n");
    files[1].second.insert(toc + std::string("toc: true\n").size(),
                           "runtime:\n  application: FExampleApplication\n"
                           "  canvas: Game\n");
    files[1].second +=
        "\n## Typed interaction\n\n```slam-ui\nColumn Calculator() {\n  Number "
        "Left(\"Left\") { value = 2; }\n  Number Right(\"Right\") { value = 3; }\n  Button "
        "Calculate(\"Add\") { on Click => Add(Left, Right) -> Result; }\n  Output "
        "Result(\"Ready\");\n};\n```\n\n"
        "\n## Canvas application\n\n```slam-ui\n"
        "Canvas Game() { width = 1280; height = 720; responsive = true; }\n```\n\n"
        "```cpp wasm\nauto Add(int32 A, int32 B) -> int32 { return A + B; }\n\n"
        "class FExampleApplication final : public FApplication {\npublic:\n"
        "  void BeginPlay() override { FLog::Information(\"Example started\"); }\n"
        "  void Tick(float DeltaTime) override { (void)DeltaTime; }\n"
        "};\n```\n";
  }
  return files;
}

auto Init(const InitOptions &options) -> Result {
  return GenerateFiles(BlankProjectFiles(), options.root, options.force);
}

auto Examples(const ExampleOptions &options) -> Result {
  return GenerateFiles(ExampleProjectFiles(options.profile), options.root,
                       options.force);
}

auto Compile(const Workspace &workspace, bool unsafeHtml) -> WorkspaceResult {
  WorkspaceResult result;
  for (const auto &file : workspace.files) {
    if (file.path.extension() != ".md" && file.path.extension() != ".slam")
      continue;
    auto document = Language::Parse(file.content, file.path);
    Language::RenderOptions options;
    options.unsafeHtml = unsafeHtml;
    if (!document.behavior.source.empty())
      options.moduleUrl =
          "/modules/" + ModuleName(file.path, file.path.parent_path()) + ".js";
    auto rendered = Language::Render(document, options);
    result.diagnostics.insert(result.diagnostics.end(), rendered.diagnostics.begin(),
                              rendered.diagnostics.end());
    if (document.metadata.draft)
      continue;
    auto output = file.path;
    output.replace_extension(".html");
    result.artifacts.push_back({std::move(output), std::move(rendered.html)});
  }
  result.success = !Language::HasErrors(result.diagnostics);
  return result;
}

auto ResolveUrl(const std::string &authored, const std::filesystem::path &source,
                const std::filesystem::path &pages, const std::string &basePath,
                bool partial) -> UrlResolution {
  UrlResolution result;
  const auto lower = Lower(authored);
  if (authored.empty() || authored.front() == '#' ||
      authored.compare(0, 2, "//") == 0 || lower.compare(0, 7, "http://") == 0 ||
      lower.compare(0, 8, "https://") == 0 || lower.compare(0, 5, "data:") == 0 ||
      lower.compare(0, 7, "mailto:") == 0 || lower.compare(0, 4, "tel:") == 0) {
    result.value = authored;
    return result;
  }
  if (HasUnknownScheme(authored) || HasEncodedTraversal(authored)) {
    result.error = "Unsafe or encoded URL.";
    return result;
  }
  const auto suffix = authored.find_first_of("?#");
  const auto pathText =
      suffix == std::string::npos ? authored : authored.substr(0, suffix);
  const auto tail = suffix == std::string::npos ? "" : authored.substr(suffix);
  auto target = (source.parent_path() / pathText).lexically_normal();
  const auto relative = std::filesystem::absolute(target).lexically_relative(
      std::filesystem::absolute(pages).lexically_normal());
  if (relative.empty() || *relative.begin() == "..") {
    result.error = "URL escapes the pages directory.";
    return result;
  }
  if (partial && (target.extension() == ".md" || target.extension() == ".slam")) {
    result.error = "Partial links must target a page, not another partial.";
    return result;
  }
  auto route = relative;
  route.replace_extension();
  if (route.filename() == "index")
    route = route.parent_path();
  result.value = NormalizeBasePath(basePath) + route.generic_string();
  if (!result.value.empty() && result.value.back() != '/')
    result.value.push_back('/');
  result.value += tail;
  result.target = std::move(target);
  result.local = true;
  return result;
}

auto Check(const std::filesystem::path &target, bool strict) -> Result {
  Result result;
  if (std::filesystem::is_regular_file(target)) {
    std::string source;
    if (!Read(target, source)) {
      result.diagnostics.push_back(
          Diagnostic(target, "SH4006", "Could not read source."));
      return result;
    }
    const auto document = Language::Parse(source, target);
    const auto rendered = Language::Render(document);
    result.diagnostics = rendered.diagnostics;
    result.success = !Language::HasErrors(result.diagnostics);
    return result;
  }
  Options options;
  options.root = target;
  options.strict = strict;
  options.dryRun = true;
  return Build(options);
}

auto Build(const Options &options) -> Result {
  Result result;
  const auto root = std::filesystem::absolute(options.root).lexically_normal();
  const auto pages = root / "pages";
  const auto partials = root / "partials";
  const auto assets = root / "assets";
  const auto code = root / "code";
  Config config;
  auto configDiagnostics = LoadConfig(root / "slamhaus.yaml", config);
  result.diagnostics.insert(result.diagnostics.end(), configDiagnostics.begin(),
                            configDiagnostics.end());
  if (!std::filesystem::is_directory(pages))
    result.diagnostics.push_back(Diagnostic(pages, "SH4004", "pages/ is missing."));
  for (const auto &style : config.styles) {
    const auto path = (root / style).lexically_normal();
    const auto relative = path.lexically_relative(assets);
    if (relative.empty() || *relative.begin() == ".." ||
        !std::filesystem::is_regular_file(path))
      result.diagnostics.push_back(Diagnostic(
          root / "slamhaus.yaml", "SH4029",
          "Theme stylesheet must be an existing file below assets/: " + style));
  }
  for (const auto &[role, name] : std::vector<std::pair<std::string, std::string>>{
           {"header", config.header},
           {"navigation", config.navigation},
           {"footer", config.footer}}) {
    if (!name.empty() && FindPartial(partials, name).empty())
      result.diagnostics.push_back(
          Diagnostic(root / "slamhaus.yaml", "SH4027",
                     "Layout " + role + " partial does not exist: " + name));
  }
  const auto sources = SourceFiles(pages);
  if (sources.empty())
    result.diagnostics.push_back(
        Diagnostic(pages, "SH4005", "No .md or .slam pages were found."));

  std::vector<ParsedPage> parsed;
  std::map<std::filesystem::path, std::filesystem::path> routes;
  std::map<std::filesystem::path, std::filesystem::path> owners;
  for (const auto &source : sources) {
    std::string content;
    if (!Read(source, content)) {
      result.diagnostics.push_back(
          Diagnostic(source, "SH4006", "Could not read page."));
      continue;
    }
    auto document = Language::Parse(content, source, MetadataDefaults(config));
    result.diagnostics.insert(result.diagnostics.end(), document.diagnostics.begin(),
                              document.diagnostics.end());
    auto route = PageRoute(source, pages, document.metadata, config.prettyUrls,
                           result.diagnostics);
    if (!route.empty() && !owners.emplace(route, source).second)
      result.diagnostics.push_back(
          Diagnostic(source, "SH4012",
                     "Two pages generate route " + route.generic_string() + "."));
    routes[std::filesystem::absolute(source).lexically_normal()] = route;
    parsed.push_back(
        {source, std::move(route), ModuleName(source, pages), std::move(document)});
  }

  for (auto &page : parsed) {
    page.document.nodes = ExpandPartials(std::move(page.document.nodes), partials,
                                         page.document.metadata, result.diagnostics);
    if (!page.document.behavior.source.empty())
      ValidateBehavior(page, result.diagnostics);
  }

  // Checking traverses the rendered graph as well as the syntax tree so broken
  // links and unsafe HTML cannot hide behind a dry run or an unrelated error.
  for (const auto &page : parsed) {
    if (page.document.metadata.draft)
      continue;
    std::vector<Language::Diagnostic> validationDiagnostics;
    Language::RenderOptions validationOptions;
    validationOptions.unsafeHtml = config.unsafeHtml;
    validationOptions.resolveUrl = [&](const std::string &url) {
      return ResolvePageUrl(url, page.source, root, routes, config,
                            validationDiagnostics);
    };
    auto validationDocument = page.document;
    validationDocument.diagnostics.clear();
    const auto rendered = Language::Render(validationDocument, validationOptions);
    result.diagnostics.insert(result.diagnostics.end(), rendered.diagnostics.begin(),
                              rendered.diagnostics.end());
    result.diagnostics.insert(result.diagnostics.end(), validationDiagnostics.begin(),
                              validationDiagnostics.end());
  }

  auto output = options.output.empty() ? root / config.output
                                       : std::filesystem::absolute(options.output);
  output = output.lexically_normal();
  if (ProtectedOutput(root, output))
    result.diagnostics.push_back(
        Diagnostic(output, "SH4009", "Refusing to replace a protected directory."));
  if (Language::HasErrors(result.diagnostics))
    return result;

  if (options.dryRun) {
    for (const auto &page : parsed)
      if (!page.document.metadata.draft)
        result.outputs.push_back(output / page.route);
    result.success = true;
    return result;
  }

  auto staging = output;
  staging += ".slamhaus-tmp";
  auto backup = output;
  backup += ".slamhaus-old";
  std::error_code error;
  std::filesystem::remove_all(staging, error);
  std::filesystem::create_directories(staging);
  Write(staging / "slamhaus.css", BaseCss());
  result.outputs.push_back(staging / "slamhaus.css");
  CopyTree(assets, staging / "assets", result, AssetIgnorePatterns(root));

  bool interactive = false;
  for (auto &page : parsed) {
    if (page.document.metadata.draft)
      continue;
    std::vector<Language::Diagnostic> renderDiagnostics;
    Language::RenderOptions renderOptions;
    renderOptions.unsafeHtml = config.unsafeHtml;
    renderOptions.runtimeUrl = config.basePath + "slamhaus-runtime.js";
    renderOptions.moduleUrl = !page.document.behavior.source.empty()
                                  ? config.basePath + "modules/" + page.module + ".js"
                              : page.document.metadata.runtime.module.empty()
                                  ? ""
                                  : config.basePath + "modules/" +
                                        page.document.metadata.runtime.module + ".js";
    renderOptions.resolveUrl = [&](const std::string &url) {
      return ResolvePageUrl(url, page.source, root, routes, config, renderDiagnostics);
    };
    auto rendered = Language::Render(page.document, renderOptions);
    result.diagnostics.insert(result.diagnostics.end(), rendered.diagnostics.begin(),
                              rendered.diagnostics.end());
    result.diagnostics.insert(result.diagnostics.end(), renderDiagnostics.begin(),
                              renderDiagnostics.end());
    const auto header = RenderPartial(config.header, partials, page.document.metadata,
                                      renderOptions, result.diagnostics);
    const auto navigation =
        RenderPartial(config.navigation, partials, page.document.metadata,
                      renderOptions, result.diagnostics);
    const auto footer = RenderPartial(config.footer, partials, page.document.metadata,
                                      renderOptions, result.diagnostics);
    const auto publicRoute = PublicRoute(page.route, config.basePath);
    const auto canonical = page.document.metadata.canonical.empty()
                               ? (config.url.empty() ? "" : config.url + publicRoute)
                               : page.document.metadata.canonical;
    const auto html = PageHtml(page.document, config, rendered.html, header, navigation,
                               footer, canonical);
    if (!Write(staging / page.route, html))
      result.diagnostics.push_back(
          Diagnostic(staging / page.route, "SH4003", "Could not write page."));
    else
      result.outputs.push_back(staging / page.route);
    interactive = interactive || !page.document.behavior.source.empty() ||
                  !page.document.metadata.runtime.module.empty() ||
                  !page.document.metadata.runtime.application.empty();
  }

  const auto generatedCode = staging / ".slamhaus-code";
  CopyTree(code, generatedCode, result);
  for (const auto &page : parsed) {
    if (page.document.behavior.source.empty())
      continue;
    Write(generatedCode / (page.module + ".cpp"), GeneratedCpp(page));
    nlohmann::json manifest = {
        {"abi", 1},
        {"language", "cpp"},
        {"module", page.module},
        {"exports", page.document.behavior.exports},
        {"application", page.document.metadata.runtime.application}};
    Write(staging / "modules" / (page.module + ".module.json"),
          manifest.dump(2) + "\n");
  }
  if (std::filesystem::is_directory(generatedCode) && !options.noWasm) {
    auto wasmDiagnostics = Wasm::Compile(generatedCode, staging / "modules");
    result.diagnostics.insert(result.diagnostics.end(), wasmDiagnostics.begin(),
                              wasmDiagnostics.end());
  } else if (interactive && options.noWasm) {
    result.diagnostics.push_back(Diagnostic(
        root, "SH3003", "Interactive source cannot be built with --no-wasm."));
  }
  std::filesystem::remove_all(generatedCode, error);
  result.outputs.erase(std::remove_if(result.outputs.begin(), result.outputs.end(),
                                      [&](const auto &path) {
                                        return path.string().compare(
                                                   0, generatedCode.string().size(),
                                                   generatedCode.string()) == 0;
                                      }),
                       result.outputs.end());
  if (interactive)
    Write(staging / "slamhaus-runtime.js", std::string(Wasm::Runtime()));

  if (config.pwa) {
    Write(staging / "manifest.webmanifest",
          nlohmann::json({{"name", config.name},
                          {"short_name", config.name},
                          {"start_url", config.basePath},
                          {"display", "standalone"},
                          {"theme_color", "#111111"},
                          {"background_color", "#ffffff"}})
                  .dump(2) +
              "\n");
    Write(staging / "slamhaus-pwa.js",
          "if('serviceWorker'in navigator)navigator.serviceWorker.register('" +
              config.basePath + "service-worker.js');\n");
    Write(staging / "service-worker.js",
          "const C='slamhaus-v1';self.addEventListener('install',e=>e.waitUntil("
          "caches.open(C).then(c=>c.addAll(['" +
              config.basePath +
              "']))));self.addEventListener('fetch',e=>e.respondWith("
              "caches.match(e.request).then(r=>r||fetch(e.request))));\n");
  }

  std::ostringstream sitemap;
  sitemap << "<?xml version=\"1.0\" encoding=\"UTF-8\"?><urlset "
             "xmlns=\"http://www.sitemaps.org/schemas/sitemap/0.9\">\n";
  for (const auto &page : parsed)
    if (!page.document.metadata.draft && !page.document.metadata.noindex)
      sitemap << "<url><loc>"
              << Language::EscapeHtml(config.url +
                                      PublicRoute(page.route, config.basePath))
              << "</loc></url>\n";
  sitemap << "</urlset>\n";
  Write(staging / "sitemap.xml", sitemap.str());

  if (config.feed) {
    std::ostringstream feed;
    feed << "<?xml version=\"1.0\"?><rss version=\"2.0\"><channel><title>"
         << Language::EscapeHtml(config.name) << "</title>";
    for (const auto &page : parsed)
      if (!page.document.metadata.draft && !page.document.metadata.date.empty())
        feed << "<item><title>"
             << Language::EscapeHtml(page.document.metadata.pageTitle)
             << "</title><link>"
             << Language::EscapeHtml(config.url +
                                     PublicRoute(page.route, config.basePath))
             << "</link></item>";
    feed << "</channel></rss>\n";
    Write(staging / "feed.xml", feed.str());
  }

  if (Language::HasErrors(result.diagnostics)) {
    std::filesystem::remove_all(staging, error);
    return result;
  }
  std::filesystem::remove_all(backup, error);
  error.clear();
  if (std::filesystem::exists(output))
    std::filesystem::rename(output, backup, error);
  if (error) {
    result.diagnostics.push_back(Diagnostic(
        output, "SH4008", "Could not preserve the previous output directory."));
    std::filesystem::remove_all(staging, error);
    return result;
  }
  std::filesystem::rename(staging, output, error);
  if (error) {
    if (std::filesystem::exists(backup))
      std::filesystem::rename(backup, output);
    result.diagnostics.push_back(
        Diagnostic(output, "SH4008", "Could not replace output directory."));
    return result;
  }
  std::filesystem::remove_all(backup, error);
  for (auto &path : result.outputs)
    if (path.string().compare(0, staging.string().size(), staging.string()) == 0)
      path = output / path.lexically_relative(staging);
  result.success = true;
  return result;
}

auto Doctor(const std::filesystem::path &root) -> nlohmann::json {
  const auto absolute = std::filesystem::absolute(root).lexically_normal();
  const auto compiler = Wasm::DiscoverCompiler();
  return {{"version", SLAMHAUS_VERSION},
          {"project", absolute.generic_string()},
          {"pages", std::filesystem::is_directory(absolute / "pages")},
          {"config", std::filesystem::is_regular_file(absolute / "slamhaus.yaml")},
          {"emscripten", compiler.empty() ? nlohmann::json(nullptr)
                                          : nlohmann::json(compiler.string())}};
}

} // namespace Project
