#include "core/project.hpp"
#include "version.hpp"
#include "wasm/wasm.hpp"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <fstream>

namespace {

class TemporaryDirectory {
public:
  TemporaryDirectory() {
    path =
        std::filesystem::temp_directory_path() /
        ("slamhaus-test-" +
         std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(path);
  }
  ~TemporaryDirectory() { std::filesystem::remove_all(path); }
  std::filesystem::path path;
};

auto Read(const std::filesystem::path &path) -> std::string {
  std::ifstream input(path, std::ios::binary);
  return {std::istreambuf_iterator<char>(input), {}};
}

auto Write(const std::filesystem::path &path, const std::string &value) -> void {
  std::filesystem::create_directories(path.parent_path());
  std::ofstream output(path, std::ios::binary);
  output << value;
}

auto Diagnostics(const Project::Result &result) -> std::string {
  return Language::DiagnosticsJson(result.diagnostics).dump();
}

auto StaticOptions(const std::filesystem::path &root) -> Project::Options {
  Project::Options options;
  options.root = root;
  options.noWasm = true;
  return options;
}

} // namespace

TEST_CASE("blank project is compiled in and minimal") {
  const auto files = Project::BlankProjectFiles();
  REQUIRE(files.size() == 5);
  REQUIRE(files[0].first == "slamhaus.yaml");
  REQUIRE(files[1].first == "pages/index.slam");
  for (const auto &[path, content] : files) {
    REQUIRE(path.string().find("demo") == std::string::npos);
    REQUIRE(content.find("@wasm") == std::string::npos);
    REQUIRE(content.find("config.ini") == std::string::npos);
  }
}

TEST_CASE("init protects existing files and force is explicit") {
  TemporaryDirectory directory;
  REQUIRE(Project::Init({directory.path, false}).success);
  REQUIRE(std::filesystem::is_regular_file(directory.path / "slamhaus.yaml"));
  REQUIRE(std::filesystem::is_regular_file(directory.path / "pages/index.slam"));
  REQUIRE_FALSE(std::filesystem::exists(directory.path / "dist"));
  REQUIRE_FALSE(Project::Init({directory.path, false}).success);
  REQUIRE(Project::Init({directory.path, true}).success);
}

TEST_CASE("example profiles are separate from init") {
  const auto minimal = Project::ExampleProjectFiles(Project::ExampleProfile::Minimal);
  const auto complete = Project::ExampleProjectFiles(Project::ExampleProfile::Complete);
  const auto joinedPages = [](const auto &files) {
    std::string result;
    for (const auto &file : files)
      if (!file.first.empty() && *file.first.begin() == "pages")
        result += file.second;
    return result;
  };
  REQUIRE(joinedPages(minimal).find("cpp wasm") == std::string::npos);
  REQUIRE(joinedPages(complete).find("cpp wasm") != std::string::npos);
  REQUIRE(joinedPages(complete).find("@wasm") == std::string::npos);
}

TEST_CASE("generated minimal and complete examples check cleanly") {
  TemporaryDirectory minimal;
  TemporaryDirectory complete;
  REQUIRE(Project::Examples({minimal.path, Project::ExampleProfile::Minimal, false})
              .success);
  REQUIRE(Project::Examples({complete.path, Project::ExampleProfile::Complete, false})
              .success);
  REQUIRE(Project::Check(minimal.path, true).success);
  REQUIRE(Project::Check(complete.path, true).success);
}

TEST_CASE("in memory workspace compiles without filesystem effects") {
  Project::Workspace workspace;
  workspace.basePath = "/docs/v2/";
  workspace.files = {{"index.md", "# Memory\n"},
                     {"counter.slam", "```slam-ui\nButton Go(\"Go\") => Run();\n```\n"
                                      "```cpp wasm\nauto Run() -> void {}\n```\n"}};
  const auto result = Project::Compile(workspace);
  REQUIRE(result.success);
  REQUIRE(result.artifacts.size() == 2);
  REQUIRE(result.artifacts.front().path == "index.html");
  REQUIRE(result.artifacts.front().content.find("id=\"memory\"") != std::string::npos);
  REQUIRE(result.artifacts.back().content.find("/docs/v2/modules/counter.js") !=
          std::string::npos);
}

TEST_CASE("blank project checks and builds to dist") {
  TemporaryDirectory directory;
  REQUIRE(Project::Init({directory.path}).success);
  REQUIRE(Project::Check(directory.path).success);
  const auto build = Project::Build(StaticOptions(directory.path));
  REQUIRE(build.success);
  REQUIRE(std::filesystem::is_regular_file(directory.path / "dist/index.html"));
  const auto html = Read(directory.path / "dist/index.html");
  REQUIRE(html.find("<h1 id=\"my-site\"") != std::string::npos);
  REQUIRE(html.find("<script") == std::string::npos);
}

TEST_CASE("dry run plans pages without writing output") {
  TemporaryDirectory directory;
  REQUIRE(Project::Init({directory.path}).success);
  auto options = StaticOptions(directory.path);
  options.dryRun = true;
  const auto result = Project::Build(options);
  REQUIRE(result.success);
  REQUIRE(result.outputs ==
          std::vector<std::filesystem::path>{directory.path / "dist/index.html"});
  REQUIRE_FALSE(std::filesystem::exists(directory.path / "dist"));
}

TEST_CASE("dangerous output paths are rejected") {
  TemporaryDirectory directory;
  REQUIRE(Project::Init({directory.path}).success);
  auto options = StaticOptions(directory.path);
  options.output = directory.path;
  REQUIRE_FALSE(Project::Build(options).success);
  options.output = directory.path / "pages";
  REQUIRE_FALSE(Project::Build(options).success);
  options.output = directory.path / "assets";
  REQUIRE_FALSE(Project::Build(options).success);
}

TEST_CASE("root yaml validates sections types and output containment") {
  TemporaryDirectory directory;
  REQUIRE(Project::Init({directory.path}).success);
  Write(directory.path / "slamhaus.yaml",
        "site:\n  name: Test\n  unknown: value\n"
        "build:\n  output: ../outside\n  pretty_urls: maybe\nunknown: true\n");
  const auto result = Project::Check(directory.path);
  REQUIRE_FALSE(result.success);
  REQUIRE(Diagnostics(result).find("SH4015") != std::string::npos);
  REQUIRE(Diagnostics(result).find("SH4014") != std::string::npos);
}

TEST_CASE("assets and configured styles are copied") {
  TemporaryDirectory directory;
  REQUIRE(Project::Init({directory.path}).success);
  Write(directory.path / "slamhaus.yaml",
        "site:\n  name: Assets\ntheme:\n  styles: [assets/site.css]\n");
  Write(directory.path / "assets/site.css", "body{color:red}\n");
  Write(directory.path / "assets/notes.md", "asset\n");
  const auto result = Project::Build(StaticOptions(directory.path));
  REQUIRE(result.success);
  REQUIRE(Read(directory.path / "dist/assets/site.css") == "body{color:red}\n");
  REQUIRE(Read(directory.path / "dist/assets/notes.md") == "asset\n");
  REQUIRE(Read(directory.path / "dist/index.html").find("href=\"/assets/site.css\"") !=
          std::string::npos);
}

TEST_CASE("configured themes styles and layouts validate during check") {
  TemporaryDirectory directory;
  REQUIRE(Project::Init({directory.path}).success);
  Write(directory.path / "slamhaus.yaml",
        "site:\n  name: Invalid config\n"
        "theme:\n  builtin: missing\n  styles: [outside.css, assets/missing.css]\n"
        "layout:\n  header: missing\n");
  const auto result = Project::Check(directory.path);
  REQUIRE_FALSE(result.success);
  const auto diagnostics = Diagnostics(result);
  REQUIRE(diagnostics.find("SH4027") != std::string::npos);
  REQUIRE(diagnostics.find("SH4028") != std::string::npos);
  REQUIRE(diagnostics.find("SH4029") != std::string::npos);
}

TEST_CASE("pretty page routes and base paths avoid parent traversal") {
  TemporaryDirectory directory;
  REQUIRE(Project::Init({directory.path}).success);
  Write(directory.path / "slamhaus.yaml",
        "site:\n  name: Docs\n  url: https://example.com\n  base_path: /docs/\n");
  Write(directory.path / "pages/index.slam", "# Home\n\n[About](about.md)\n");
  Write(directory.path / "pages/about.md", "# About\n\n[Home](index.slam)\n");
  REQUIRE(Project::Build(StaticOptions(directory.path)).success);
  const auto home = Read(directory.path / "dist/index.html");
  const auto about = Read(directory.path / "dist/about/index.html");
  REQUIRE(home.find("href=\"/docs/about/\"") != std::string::npos);
  REQUIRE(about.find("href=\"/docs/\"") != std::string::npos);
  REQUIRE(home.find("../") == std::string::npos);
  REQUIRE(Read(directory.path / "dist/sitemap.xml")
              .find("https://example.com/docs/about/") != std::string::npos);
}

TEST_CASE("missing links and route collisions fail validation") {
  TemporaryDirectory directory;
  REQUIRE(Project::Init({directory.path}).success);
  Write(directory.path / "pages/index.slam", "[Missing](missing.md)\n");
  Write(directory.path / "pages/page.slam", "---\npermalink: /same/\n---\n# One\n");
  Write(directory.path / "pages/page.md", "---\npermalink: /same/\n---\n# Two\n");
  const auto result = Project::Check(directory.path);
  REQUIRE_FALSE(result.success);
  REQUIRE(Diagnostics(result).find("SH4011") != std::string::npos);
  REQUIRE(Diagnostics(result).find("SH4012") != std::string::npos);
}

TEST_CASE("draft pages validate but are not emitted") {
  TemporaryDirectory directory;
  REQUIRE(Project::Init({directory.path}).success);
  Write(directory.path / "pages/draft.md", "---\ndraft: true\n---\n# Draft\n");
  REQUIRE(Project::Build(StaticOptions(directory.path)).success);
  REQUIRE_FALSE(std::filesystem::exists(directory.path / "dist/draft/index.html"));
}

TEST_CASE("permalinks control public output") {
  TemporaryDirectory directory;
  REQUIRE(Project::Init({directory.path}).success);
  Write(directory.path / "pages/guide.slam",
        "---\ntitle: Guide\npermalink: /manual/start/\n---\n# Guide\n");
  REQUIRE(Project::Build(StaticOptions(directory.path)).success);
  REQUIRE(std::filesystem::is_regular_file(directory.path /
                                           "dist/manual/start/index.html"));
}

TEST_CASE("ui partials expand and cycles are rejected") {
  TemporaryDirectory directory;
  REQUIRE(Project::Init({directory.path}).success);
  Write(directory.path / "pages/index.slam",
        "# Home\n```slam-ui\nInclude(\"card\");\n```\n");
  Write(directory.path / "partials/card.slam", "## Included\n");
  REQUIRE(Project::Build(StaticOptions(directory.path)).success);
  REQUIRE(Read(directory.path / "dist/index.html").find("id=\"included\"") !=
          std::string::npos);
  Write(directory.path / "partials/card.slam", "```slam-ui\nInclude(\"card\");\n```\n");
  const auto result = Project::Check(directory.path);
  REQUIRE_FALSE(result.success);
  REQUIRE(Diagnostics(result).find("SH4024") != std::string::npos);
}

TEST_CASE("layout partials wrap page content") {
  TemporaryDirectory directory;
  REQUIRE(Project::Init({directory.path}).success);
  Write(directory.path / "slamhaus.yaml",
        "site:\n  name: Layout\nlayout:\n  header: header\n  navigation: nav\n  "
        "footer: footer\n");
  Write(directory.path / "partials/header.md", "Site header\n");
  Write(directory.path / "partials/nav.md", "[Home](index.slam)\n");
  Write(directory.path / "partials/footer.md", "Site footer\n");
  REQUIRE(Project::Build(StaticOptions(directory.path)).success);
  const auto html = Read(directory.path / "dist/index.html");
  REQUIRE(html.find("Site header") < html.find("<main>"));
  REQUIRE(html.find("Site footer") > html.find("</main>"));
  REQUIRE(html.find("class=\"slamhaus-header\"") != std::string::npos);
  REQUIRE(html.find("class=\"slamhaus-navigation\"") != std::string::npos);
  REQUIRE(html.find("class=\"slamhaus-footer\"") != std::string::npos);
}

TEST_CASE("standalone source checks do not need a project") {
  TemporaryDirectory directory;
  const auto path = directory.path / "page.slam";
  Write(path, "# Standalone\n");
  REQUIRE(Project::Check(path).success);
}

TEST_CASE("no wasm rejects interactive projects") {
  TemporaryDirectory directory;
  REQUIRE(Project::Examples({directory.path, Project::ExampleProfile::Complete, false})
              .success);
  const auto result = Project::Build(StaticOptions(directory.path));
  REQUIRE_FALSE(result.success);
  REQUIRE(Diagnostics(result).find("SH3003") != std::string::npos);
  REQUIRE_FALSE(std::filesystem::exists(directory.path / "dist"));
}

TEST_CASE("embedded behavior validates missing functions and overloads") {
  TemporaryDirectory directory;
  REQUIRE(Project::Init({directory.path}).success);
  Write(directory.path / "pages/index.slam",
        "```slam-ui\nButton Go(\"Go\") => Missing();\n```\n"
        "```cpp wasm\nauto Other() -> void {}\n```\n");
  auto result = Project::Check(directory.path);
  REQUIRE_FALSE(result.success);
  REQUIRE(Diagnostics(result).find("SH1124") != std::string::npos);
  Write(directory.path / "pages/index.slam",
        "```slam-ui\nButton Go(\"Go\") => Run();\n```\n"
        "```cpp wasm\nauto Run() -> void {}\nauto Run(int32 X) -> void {}\n```\n");
  result = Project::Check(directory.path);
  REQUIRE_FALSE(result.success);
  REQUIRE(Diagnostics(result).find("SH1125") != std::string::npos);
}

TEST_CASE("real embedded behavior emits module and manifest when compiler exists") {
  if (Wasm::DiscoverCompiler().empty())
    return;
  TemporaryDirectory directory;
  REQUIRE(Project::Examples({directory.path, Project::ExampleProfile::Complete, false})
              .success);
  Project::Options options;
  options.root = directory.path;
  const auto result = Project::Build(options);
  INFO(Diagnostics(result));
  REQUIRE(result.success);
  REQUIRE(std::filesystem::is_regular_file(directory.path / "dist/modules/index.js"));
  REQUIRE(std::filesystem::is_regular_file(directory.path / "dist/modules/index.wasm"));
  REQUIRE(Read(directory.path / "dist/modules/index.module.json").find("\"Add\"") !=
          std::string::npos);
  const auto html = Read(directory.path / "dist/index.html");
  REQUIRE(html.find("/modules/index.js") != std::string::npos);
  REQUIRE(html.find("slamhaus-runtime.js") != std::string::npos);
  REQUIRE(html.find("data-slamhaus-bindings") == html.rfind("data-slamhaus-bindings"));
}

TEST_CASE("configured base path prefixes interactive resources") {
  if (Wasm::DiscoverCompiler().empty())
    return;
  TemporaryDirectory directory;
  REQUIRE(Project::Examples({directory.path, Project::ExampleProfile::Complete, false})
              .success);
  Write(directory.path / "slamhaus.yaml",
        "slamhaus:\n  version: 0.1.0\nsite:\n  name: Nested\n"
        "  base_path: /docs/v2/\n");
  Project::Options options;
  options.root = directory.path;
  const auto result = Project::Build(options);
  INFO(Diagnostics(result));
  REQUIRE(result.success);
  const auto html = Read(directory.path / "dist/index.html");
  REQUIRE(html.find("href=\"/docs/v2/slamhaus.css\"") != std::string::npos);
  REQUIRE(html.find("\"module\":\"/docs/v2/modules/index.js\"") !=
          std::string::npos);
  REQUIRE(html.find("src=\"/docs/v2/slamhaus-runtime.js\"") != std::string::npos);
}

TEST_CASE("runtime resolves modules relative to its own URL") {
  REQUIRE(std::string(Wasm::Runtime()).find("new URL(url,import.meta.url)") !=
          std::string::npos);
}

TEST_CASE("pwa project emits generated application files") {
  TemporaryDirectory directory;
  REQUIRE(Project::Init({directory.path}).success);
  Write(directory.path / "slamhaus.yaml",
        "site:\n  name: App\npwa:\n  enabled: true\n");
  REQUIRE(Project::Build(StaticOptions(directory.path)).success);
  REQUIRE(
      std::filesystem::is_regular_file(directory.path / "dist/manifest.webmanifest"));
  REQUIRE(std::filesystem::is_regular_file(directory.path / "dist/service-worker.js"));
  REQUIRE(Read(directory.path / "dist/index.html").find("manifest.webmanifest") !=
          std::string::npos);
}

TEST_CASE("feeds include dated pages when enabled") {
  TemporaryDirectory directory;
  REQUIRE(Project::Init({directory.path}).success);
  Write(directory.path / "slamhaus.yaml",
        "site:\n  name: News\n  url: https://example.com\nfeed:\n  enabled: true\n");
  Write(directory.path / "pages/news.md",
        "---\ntitle: Release\ndate: 2026-08-16\n---\n# Release\n");
  REQUIRE(Project::Build(StaticOptions(directory.path)).success);
  const auto feed = Read(directory.path / "dist/feed.xml");
  REQUIRE(feed.find("Release") != std::string::npos);
  REQUIRE(feed.find("https://example.com/news/") != std::string::npos);
}

TEST_CASE("failed build preserves the previous successful dist") {
  TemporaryDirectory directory;
  REQUIRE(Project::Init({directory.path}).success);
  const auto options = StaticOptions(directory.path);
  REQUIRE(Project::Build(options).success);
  const auto before = Read(directory.path / "dist/index.html");
  Write(directory.path / "pages/index.slam",
        "```slam-ui\nButton Broken(\"Broken\"\n```\n");
  REQUIRE_FALSE(Project::Build(options).success);
  REQUIRE(Read(directory.path / "dist/index.html") == before);
}

TEST_CASE("build output is reproducible") {
  TemporaryDirectory directory;
  REQUIRE(Project::Init({directory.path}).success);
  const auto options = StaticOptions(directory.path);
  REQUIRE(Project::Build(options).success);
  const auto first = Read(directory.path / "dist/index.html");
  const auto css = Read(directory.path / "dist/slamhaus.css");
  REQUIRE(Project::Build(options).success);
  REQUIRE(Read(directory.path / "dist/index.html") == first);
  REQUIRE(Read(directory.path / "dist/slamhaus.css") == css);
}

TEST_CASE("url resolution emits logical routes and blocks traversal") {
  TemporaryDirectory directory;
  const auto pages = directory.path / "pages";
  Write(pages / "guide/index.slam", "");
  Write(pages / "about.md", "# About\n");
  const auto nested = Project::ResolveUrl("../about.md?x=1#top",
                                          pages / "guide/index.slam", pages, "/docs/");
  REQUIRE(nested.error.empty());
  REQUIRE(nested.local);
  REQUIRE(nested.value == "/docs/about/?x=1#top");
  REQUIRE(nested.value.find("..") == std::string::npos);
  REQUIRE_FALSE(
      Project::ResolveUrl("../../outside.md", pages / "guide/index.slam", pages)
          .error.empty());
  REQUIRE_FALSE(
      Project::ResolveUrl("javascript:alert(1)", pages / "guide/index.slam", pages)
          .error.empty());
  REQUIRE_FALSE(
      Project::ResolveUrl("%2e%2e/about.md", pages / "guide/index.slam", pages)
          .error.empty());
}

TEST_CASE("doctor reports canonical project structure") {
  TemporaryDirectory directory;
  REQUIRE(Project::Init({directory.path}).success);
  const auto report = Project::Doctor(directory.path);
  REQUIRE(report["version"] == SLAMHAUS_VERSION);
  REQUIRE(report["pages"] == true);
  REQUIRE(report["config"] == true);
  REQUIRE(report.contains("emscripten"));
}
