#include "util/process.hpp"

#include <cstdlib>
#include <sstream>

#ifdef _WIN32
#include <process.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace Process {
namespace {

auto SplitPath(const char *value) -> std::vector<std::filesystem::path> {
  std::vector<std::filesystem::path> paths;
  if (value == nullptr)
    return paths;
  std::istringstream input(value);
  std::string item;
#ifdef _WIN32
  constexpr char separator = ';';
#else
  constexpr char separator = ':';
#endif
  while (std::getline(input, item, separator)) {
    if (!item.empty())
      paths.emplace_back(item);
  }
  return paths;
}

} // namespace

auto FindExecutable(const std::string &name) -> std::filesystem::path {
  std::filesystem::path requested(name);
  if (requested.has_parent_path() && std::filesystem::exists(requested)) {
    return requested;
  }
  for (const auto &directory : SplitPath(std::getenv("PATH"))) {
    auto candidate = directory / name;
    if (std::filesystem::is_regular_file(candidate))
      return candidate;
#ifdef _WIN32
    candidate += ".exe";
    if (std::filesystem::is_regular_file(candidate))
      return candidate;
#endif
  }
  return {};
}

auto Run(const std::filesystem::path &executable,
         const std::vector<std::string> &arguments) -> int {
  std::vector<std::string> storage;
  storage.reserve(arguments.size() + 1);
  storage.push_back(executable.string());
  storage.insert(storage.end(), arguments.begin(), arguments.end());
  std::vector<char *> argv;
  argv.reserve(storage.size() + 1);
  for (auto &argument : storage)
    argv.push_back(argument.data());
  argv.push_back(nullptr);
#ifdef _WIN32
  return static_cast<int>(_spawnv(_P_WAIT, executable.string().c_str(), argv.data()));
#else
  const pid_t process = fork();
  if (process == 0) {
    execv(executable.c_str(), argv.data());
    _exit(127);
  }
  if (process < 0)
    return -1;
  int status = 0;
  if (waitpid(process, &status, 0) < 0)
    return -1;
  return WIFEXITED(status) ? WEXITSTATUS(status) : 128;
#endif
}

} // namespace Process
