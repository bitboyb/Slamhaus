#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace Process {

/** Find an executable using the host PATH, or return an empty path. */
auto FindExecutable(const std::string &name) -> std::filesystem::path;
/** Run a process synchronously and return its exit status. */
auto Run(const std::filesystem::path &executable,
         const std::vector<std::string> &arguments) -> int;

} // namespace Process
