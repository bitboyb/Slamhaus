#include "asset.hpp"
#include <filesystem>
#include <iostream>

namespace Asset {
auto CopyAssets(const std::string &sourceDir, const std::string &destDir)
    -> bool {
  try {
    if (!std::filesystem::exists(sourceDir)) {
      return true;
    }
    std::filesystem::create_directories(destDir);
    for (const auto &entry :
         std::filesystem::recursive_directory_iterator(sourceDir)) {
      const auto &srcPath = entry.path();
      auto relPath = std::filesystem::relative(srcPath, sourceDir);
      auto destPath = std::filesystem::path(destDir) / relPath;

      if (entry.is_directory()) {
        std::filesystem::create_directories(destPath);
        continue;
      }

      if (!entry.is_regular_file()) {
        continue;
      }

      std::filesystem::create_directories(destPath.parent_path());
      std::filesystem::copy(srcPath, destPath,
                            std::filesystem::copy_options::overwrite_existing);
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
