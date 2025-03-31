#include "asset.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

void CopyAssets(const std::string &sourceDir, const std::string &destDir)
{
    try 
    {
        if (!fs::exists(sourceDir)) 
        {
            std::cerr << "Source directory does not exist: " << sourceDir << std::endl;
            return;
        }
        fs::create_directories(destDir);
        for (const auto &entry : fs::recursive_directory_iterator(sourceDir)) 
        {
            const auto &srcPath = entry.path();
            auto relPath = fs::relative(srcPath, sourceDir);
            auto destPath = fs::path(destDir) / relPath;
            fs::create_directories(destPath.parent_path());
            fs::copy(srcPath, destPath, fs::copy_options::overwrite_existing);
            std::cout << "Copied: " << srcPath << " -> " << destPath << std::endl;
        }
    } 
    catch (const fs::filesystem_error &e) 
    {
        std::cerr << "Error copying assets: " << e.what() << std::endl;
    }
}
