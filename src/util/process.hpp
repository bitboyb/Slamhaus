#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace Process {

auto FindExecutable(const std::string &name) -> std::filesystem::path;
auto Run(const std::filesystem::path &executable,
         const std::vector<std::string> &arguments) -> int;

} // namespace Process
