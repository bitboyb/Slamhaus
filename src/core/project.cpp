#include "project.hpp"
#include <filesystem>
#include <iostream>

namespace Project {
auto Resolve(const std::filesystem::path &projectRoot) -> Layout {
  Layout layout;
  layout.projectRoot = std::filesystem::absolute(projectRoot);
  layout.contentDir = layout.projectRoot / "content";
  layout.outputDir = layout.projectRoot / "output";
  layout.assetsDir = layout.contentDir / "assets";
  layout.themeDir = layout.assetsDir / "theme";
  layout.scriptsDir = layout.contentDir / "scripts";
  return layout;
}

auto CopyProjectScripts(const Layout &layout) -> bool {
  namespace fs = std::filesystem;

  try {
    if (!fs::exists(layout.scriptsDir) || !fs::is_directory(layout.scriptsDir)) {
      return true;
    }

    auto outputScriptsDir = layout.outputDir / "scripts";
    fs::create_directories(outputScriptsDir);

    for (const auto &entry : fs::recursive_directory_iterator(layout.scriptsDir)) {
      auto relPath = fs::relative(entry.path(), layout.scriptsDir);
      auto destPath = outputScriptsDir / relPath;

      if (entry.is_directory()) {
        fs::create_directories(destPath);
        continue;
      }

      if (!entry.is_regular_file()) {
        continue;
      }

      auto ext = entry.path().extension().string();
      if (ext == ".cpp" || ext == ".hpp" || ext == ".h") {
        continue;
      }

      fs::create_directories(destPath.parent_path());
      fs::copy_file(entry.path(), destPath,
                    fs::copy_options::overwrite_existing);
    }
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Error copying project scripts: " << e.what() << "\n";
    return false;
  }
}
} // namespace Project
