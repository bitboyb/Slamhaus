#include <algorithm>
#include <string>

namespace String {
auto Trim(const std::string &str) -> std::string {
  const auto strBegin = std::find_if_not(str.begin(), str.end(), ::isspace);
  const auto strEnd =
      std::find_if_not(str.rbegin(), str.rend(), ::isspace).base();
  return (strBegin < strEnd) ? std::string(strBegin, strEnd) : "";
}
} // namespace String