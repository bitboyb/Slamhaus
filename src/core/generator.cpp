#include "generator.hpp"
#include "../wasm/wasm.hpp"
#include "../parser/parser.hpp"
#include "../parser/seo.hpp"
#include "asset.hpp"
#include "config.hpp"
#include "meta.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include "template.hpp"
#include "../util/html.hpp"
#include "css.hpp"


namespace Generator 
{    
    void GenerateSite(const std::string &contentDir, 
                      const std::string &outputDir)
    {
        if (!std::filesystem::exists(contentDir) || !std::filesystem::is_directory(contentDir))
        {
            std::cerr << "Content directory not found: " << contentDir << "\n";
            return;
        }
        if (std::filesystem::exists(outputDir))
        {
            std::cout
                << "Output directory \"" << outputDir
                << "\" already exists and will be deleted. Continue? [y/N] ";
            char confirm = 'n';
            std::cin  >> confirm;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (confirm != 'y' && confirm != 'Y')
            {
                std::cout << "Aborting build.\n";
                return;
            }
            std::filesystem::remove_all(outputDir);
        }
        std::filesystem::create_directories(outputDir);
        std::string cssContent = CSS::LoadCSS(contentDir + "/assets/theme");
        Config::ConfigINI ini = Config::GetConfig(contentDir);
        std::vector<std::string> generatedPages;
        std::string templateString = Template::BuildTemplate(ini, contentDir);
        Asset::CopyAssets(contentDir + "/assets/", outputDir + "/assets");
        for (const auto &entry : std::filesystem::recursive_directory_iterator(contentDir))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".cpp")
            {
                std::filesystem::path relPath = std::filesystem::relative(entry.path(), contentDir);
                std::filesystem::path outBase = std::filesystem::path(outputDir) / relPath.parent_path() / relPath.stem();
                WASM::WASMFile wasmFile 
                {
                    entry.path().string(),
                    outBase.string()
                };
                std::cout << "Compiling WASM: " << wasmFile.inputPath << "\n";
                bool success = WASM::Compile({ wasmFile }, "emcc");
                if (!success)
                {
                    std::cerr << "Failed to compile WASM: " << wasmFile.inputPath << "\n";
                }
            }
            else if (entry.is_regular_file() && entry.path().extension() == ".md")
            {
                std::string filename = entry.path().filename().string();
                if (filename == "header.md" || filename == "nav.md"    || filename == "footer.md")
                {
                    std::cout << "Skipping partial file: " << entry.path() << std::endl;
                    continue;
                }
                std::filesystem::path relPath = std::filesystem::relative(entry.path(), contentDir);
                relPath.replace_extension(".html");
                std::filesystem::path outputPath = std::filesystem::path(outputDir) / relPath;
                std::filesystem::create_directories(outputPath.parent_path());
                std::ifstream mdFile(entry.path());
                if (!mdFile)
                {
                    std::cerr << "Error opening Markdown file: " << entry.path() << std::endl;
                    continue;
                }
                std::string markdown((std::istreambuf_iterator<char>(mdFile)),
                                     std::istreambuf_iterator<char>());
                
                SEO::SEOData seo = SEO::GetDefault(ini);
                std::string htmlContent = Parser::ParseMarkdown(markdown, &seo);

                std::string finalHtml = Template::ApplyTemplateFromString(htmlContent, templateString, cssContent, seo);
                finalHtml = HTML::AdjustLinks(finalHtml);
                std::ofstream outFile(outputPath);
                if (!outFile)
                {
                    std::cerr << "Error creating output file: " << outputPath << std::endl;
                    continue;
                }
                outFile << finalHtml;
                std::cout << "Generated: " << outputPath << std::endl;
                std::string htmlFilename = relPath.filename().string();
                generatedPages.push_back(relPath.generic_string());

            }
        }
        Meta::GenerateSiteMetaFiles(ini.siteName, cssContent, outputDir, generatedPages);
    }    
}

