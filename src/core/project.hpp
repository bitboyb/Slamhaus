#pragma once

#ifndef PROJECT_HPP
#define PROJECT_HPP

#include <filesystem>

namespace Project {
/// Describes the standard Slamhaus project layout.
struct Layout {
  std::filesystem::path projectRoot;
  std::filesystem::path contentDir;
  std::filesystem::path outputDir;
  std::filesystem::path assetsDir;
  std::filesystem::path themeDir;
  std::filesystem::path scriptsDir;
};

/// Resolves the standard `content/`, `output/`, `assets/`, and `scripts/`
/// paths for a Slamhaus project root.
///
/// @param projectRoot Root directory containing the Slamhaus project
/// @return A populated project layout description
auto Resolve(const std::filesystem::path &projectRoot) -> Layout;

/// Copies non-C++ project script assets from `content/scripts` into
/// `output/scripts`.
///
/// @param layout The project layout to copy from
/// @return True if the copy completed successfully
auto CopyProjectScripts(const Layout &layout) -> bool;
} // namespace Project

#endif // PROJECT_HPP
