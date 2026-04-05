#include "generator.hpp"
#include "../parser/parser.hpp"
#include "../parser/seo.hpp"
#include "../util/html.hpp"
#include "../util/log.hpp"
#include "../util/smd.hpp"
#include "../wasm/wasm.hpp"
#include "asset.hpp"
#include "constants.hpp"
#include "css.hpp"
#include "meta.hpp"
#include "project.hpp"
#include "template.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>

namespace Generator {
auto RenderPage(const std::string &markdown, const std::string &templateString,
                const std::string &stylesheetTags,
                const std::string &faviconHref,
                const Config::ConfigINI &ini) -> PageOutput {
  SEO::SEOData seo = SEO::GetDefault(ini);
  std::string htmlContent = Parser::ParseMarkdown(markdown, &seo);
  std::string finalHtml = Template::ApplyTemplateFromString(
      htmlContent, templateString, stylesheetTags, faviconHref, seo);
  finalHtml = HTML::AdjustLinks(finalHtml);
  return {finalHtml};
}

auto GenerateSite(const std::filesystem::path &contentDir,
                  const std::filesystem::path &outputDir, bool skipConfirmation,
                  bool quiet) -> bool {
  if (!std::filesystem::exists(contentDir) ||
      !std::filesystem::is_directory(contentDir)) {
    std::cerr << "Content directory not found: " << contentDir << "\n";
    return false;
  }

  if (std::filesystem::exists(outputDir)) {
    if (!skipConfirmation) {
      if (!quiet) {
        std::cout << "Output directory \"" << outputDir
                  << "\" already exists and will be deleted. Continue? [y/N] ";
      }

      char confirm = 'n';
      std::cin >> confirm;
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      if (confirm != 'y' && confirm != 'Y') {
        if (!quiet) {
          std::cout << "Aborting build.\n";
        }
        return false;
      }
    }

    std::filesystem::remove_all(outputDir);
  }

  std::filesystem::create_directories(outputDir);

  std::filesystem::path htaccessPath = contentDir / Constants::kFileHtaccess;
  if (std::filesystem::exists(htaccessPath)) {
    std::filesystem::copy_file(
        htaccessPath, outputDir / Constants::kFileHtaccess,
        std::filesystem::copy_options::overwrite_existing);
    if (!quiet) {
      Log::Info("Copied .htaccess to output.");
    }
  }

  Config::ConfigINI ini = Config::GetConfig(contentDir.string());
  auto stylesheets = CSS::CollectStylesheets(contentDir / "assets/theme");
  std::string templateString =
      Template::BuildTemplate(ini, contentDir.string());
  std::vector<std::string> generatedPages;
  bool wasmCompilerAvailable = WASM::IsCompilerAvailable("emcc");
  bool warnedAboutMissingWasmCompiler = false;

  if (!Asset::CopyAssets((contentDir / "assets").string(),
                         (outputDir / "assets").string())) {
    Log::Error("Failed to copy assets.");
    return false;
  }

  Project::Layout layout;
  layout.contentDir = contentDir;
  layout.outputDir = outputDir;
  layout.scriptsDir = contentDir / "scripts";
  if (!Project::CopyProjectScripts(layout)) {
    Log::Error("Failed to copy project scripts.");
    return false;
  }

  for (const auto &entry :
       std::filesystem::recursive_directory_iterator(contentDir)) {
    if (!entry.is_regular_file()) {
      continue;
    }

    std::filesystem::path relPath =
        std::filesystem::relative(entry.path(), contentDir);

    if (!relPath.empty() && (*relPath.begin() == "scripts") &&
        WASM::IsSourceFile(entry.path())) {
      if (!wasmCompilerAvailable) {
        if (!warnedAboutMissingWasmCompiler) {
          Log::Error("Emscripten not found. Skipping WASM compilation.");
          warnedAboutMissingWasmCompiler = true;
        }
        continue;
      }

      std::filesystem::path outBase =
          std::filesystem::path(outputDir) / relPath.parent_path() /
          relPath.stem();
      WASM::WASMFile wasmFile{entry.path().string(), outBase.string()};

      if (!quiet) {
        std::cout << "Compiling WASM: " << wasmFile.inputPath << "\n";
      }

      if (!WASM::Compile({wasmFile}, "emcc")) {
        std::cerr << "Failed to compile WASM: " << wasmFile.inputPath << "\n";
        return false;
      }
      continue;
    }

    if (!SMD::IsContentFile(entry.path())) {
      continue;
    }

    std::string ext = entry.path().extension().string();
    if (ext == Constants::kExtSmd) {
      if (!quiet) {
        std::cout << "Skipping partial file: " << entry.path() << std::endl;
      }
      continue;
    }

    std::filesystem::path outputRelPath = relPath;
    outputRelPath.replace_extension(".html");
    std::filesystem::path outputPath =
        std::filesystem::path(outputDir) / outputRelPath;
    std::filesystem::create_directories(outputPath.parent_path());

    std::ifstream mdFile(entry.path());
    if (!mdFile) {
      std::cerr << "Error opening content file: " << entry.path() << std::endl;
      continue;
    }

    std::string markdown((std::istreambuf_iterator<char>(mdFile)),
                         std::istreambuf_iterator<char>());
    std::string pagePrefix = HTML::GetRelativeRoot(outputRelPath);
    std::string stylesheetTags =
        CSS::BuildStylesheetTags(stylesheets, pagePrefix);
    std::string faviconHref = HTML::PrefixPath(ini.faviconPath, pagePrefix);
    PageOutput page =
        RenderPage(markdown, templateString, stylesheetTags, faviconHref, ini);

    std::ofstream outFile(outputPath);
    if (!outFile) {
      std::cerr << "Error creating output file: " << outputPath << std::endl;
      continue;
    }

    outFile << page.html;
    if (!quiet) {
      std::cout << "Generated: " << outputPath << std::endl;
    }
    generatedPages.push_back(outputRelPath.generic_string());
  }

  Meta::GenerateSiteMetaFiles(ini.siteUrl,
                              CSS::BuildStylesheetTags(stylesheets, ""),
                              outputDir.string(), generatedPages);
  return true;
}

auto GenerateProject(const std::filesystem::path &projectRoot,
                     bool skipConfirmation, bool quiet) -> bool {
  auto layout = Project::Resolve(projectRoot);
  return GenerateSite(layout.contentDir, layout.outputDir, skipConfirmation,
                      quiet);
}
} // namespace Generator
