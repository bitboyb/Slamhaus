#include "io.hpp"
#include <fstream>
#include <sstream>

namespace Util {
auto ReadFile(const std::filesystem::path &path) -> std::string {
  std::ifstream file(path);
  if (!file) {
    return "";
  }
  std::ostringstream ss;
  ss << file.rdbuf();
  return ss.str();
}
} // namespace Util
