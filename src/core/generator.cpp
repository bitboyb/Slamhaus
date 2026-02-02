#include "generator.hpp"
#include "../parser/parser.hpp"
#include "../parser/seo.hpp"
#include "../util/html.hpp"
#include "../util/log.hpp"
#include "../util/smd.hpp"
#include "../wasm/wasm.hpp"
#include "asset.hpp"
#include "config.hpp"
#include "constants.hpp"
#include "css.hpp"
#include "meta.hpp"
#include "template.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace Generator {
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
    if (!quiet)
      Log::Info("Copied .htaccess to output.");
  }
  std::string cssContent = CSS::LoadCSS((contentDir / "assets/theme").string());
  Config::ConfigINI ini = Config::GetConfig(contentDir.string());
  std::vector<std::string> generatedPages;
  std::string templateString =
      Template::BuildTemplate(ini, contentDir.string());
  if (!Asset::CopyAssets((contentDir / "assets/").string(),
                         (outputDir / "assets").string())) {
    Log::Error("Failed to copy assets.");
    return false;
  }
  for (const auto &entry :
       std::filesystem::recursive_directory_iterator(contentDir)) {
    if (entry.is_regular_file() && WASM::IsSourceFile(entry.path())) {
      std::filesystem::path relPath =
          std::filesystem::relative(entry.path(), contentDir);
      std::filesystem::path outBase = std::filesystem::path(outputDir) /
                                      relPath.parent_path() / relPath.stem();
      WASM::WASMFile wasmFile{entry.path().string(), outBase.string()};
      if (!quiet) {
        std::cout << "Compiling WASM: " << wasmFile.inputPath << "\n";
      }
      bool success = WASM::Compile({wasmFile}, "emcc");
      if (!success) {
        std::cerr << "Failed to compile WASM: " << wasmFile.inputPath << "\n";
        return false;
      }
    } else if (entry.is_regular_file() && SMD::IsContentFile(entry.path())) {
      std::string ext = entry.path().extension().string();
      if (ext == Constants::kExtSmd) {
        if (!quiet)
          std::cout << "Skipping partial file: " << entry.path() << std::endl;
        continue;
      }

      std::filesystem::path relPath =
          std::filesystem::relative(entry.path(), contentDir);
      relPath.replace_extension(".html");
      std::filesystem::path outputPath =
          std::filesystem::path(outputDir) / relPath;
      std::filesystem::create_directories(outputPath.parent_path());

      std::ifstream mdFile(entry.path());
      if (!mdFile) {
        std::cerr << "Error opening content file: " << entry.path()
                  << std::endl;
        continue;
      }

      std::string markdown((std::istreambuf_iterator<char>(mdFile)),
                           std::istreambuf_iterator<char>());

      SEO::SEOData seo = SEO::GetDefault(ini);
      std::string htmlContent = Parser::ParseMarkdown(markdown, &seo);

      std::string finalHtml = Template::ApplyTemplateFromString(
          htmlContent, templateString, cssContent, seo);
      finalHtml = HTML::AdjustLinks(finalHtml);

      std::ofstream outFile(outputPath);
      if (!outFile) {
        std::cerr << "Error creating output file: " << outputPath << std::endl;
        continue;
      }
      outFile << finalHtml;
      if (!quiet)
        std::cout << "Generated: " << outputPath << std::endl;
      generatedPages.push_back(relPath.generic_string());
    }
  }
  Meta::GenerateSiteMetaFiles(ini.siteName, cssContent, outputDir,
                              generatedPages);
  return true;
}
} // namespace Generator
