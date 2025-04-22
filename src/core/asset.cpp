#include "asset.hpp"
#include <filesystem>
#include <iostream>

namespace Asset 
{
    void CopyAssets(const std::string &sourceDir, 
                    const std::string &destDir)
    {
        try 
        {
            if (!std::filesystem::exists(sourceDir)) 
            {
                std::cerr << "Source directory does not exist: " << sourceDir << std::endl;
                return;
            }
            std::filesystem::create_directories(destDir);
            for (const auto &entry : std::filesystem::recursive_directory_iterator(sourceDir)) 
            {
                const auto &srcPath = entry.path();
                if (entry.is_regular_file() && srcPath.extension() == ".css")
                {
                    std::cout << "Skipping .css file: " << srcPath << std::endl;
                    continue;
                }
                auto relPath = std::filesystem::relative(srcPath, sourceDir);
                auto destPath = std::filesystem::path(destDir) / relPath;
                std::filesystem::create_directories(destPath.parent_path());
                std::filesystem::copy(srcPath, destPath, std::filesystem::copy_options::overwrite_existing);
                std::cout << "Copied: " << srcPath << " -> " << destPath << std::endl;
            }
        } 
        catch (const std::filesystem::filesystem_error &e) 
        {
            std::cerr << "Error copying assets: " << e.what() << std::endl;
        }
    }
}

