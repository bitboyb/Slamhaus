#include "asset.hpp"
#include <filesystem>
#include <iostream>

namespace Asset {
auto CopyAssets(const std::string &sourceDir, const std::string &destDir)
    -> bool {
  try {
    if (!std::filesystem::exists(sourceDir)) {
      std::cerr << "Source directory does not exist: " << sourceDir
                << std::endl;
      // User "Avoid silent failures". If source dir missing, it's likely a
      // config error or user error. The original code returned (void) after
      // logging. I will return false to be explicit.
      return false;
    }
    std::filesystem::create_directories(destDir);
    for (const auto &entry :
         std::filesystem::recursive_directory_iterator(sourceDir)) {
      const auto &srcPath = entry.path();
      if (entry.is_regular_file() && srcPath.extension() == ".css") {
        std::cout << "Skipping .css file: " << srcPath << std::endl;
        continue;
      }
      auto relPath = std::filesystem::relative(srcPath, sourceDir);
      auto destPath = std::filesystem::path(destDir) / relPath;
      std::filesystem::create_directories(destPath.parent_path());
      std::filesystem::copy(srcPath, destPath,
                            std::filesystem::copy_options::overwrite_existing);
      std::cout << "Copied: " << srcPath << " -> " << destPath << std::endl;
    }
    return true;
  } catch (const std::filesystem::filesystem_error &e) {
    std::cerr << "Error copying assets: " << e.what() << std::endl;
    return false;
  } catch (const std::exception &e) {
    std::cerr << "Error in CopyAssets: " << e.what() << "\n";
    return false;
  }
}
} // namespace Asset
