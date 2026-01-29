#include "smd.hpp"
#include "../core/constants.hpp"
#include <set>

namespace SMD {
bool IsContentFile(const std::filesystem::path &path) {
  static const std::set<std::string> validExtensions = {Constants::kExtSmd,
                                                        Constants::kExtSlam};
  return path.has_extension() &&
         validExtensions.count(path.extension().string()) > 0;
}
} // namespace SMD
