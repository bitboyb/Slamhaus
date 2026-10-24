#include "toolchain/toolchain.hpp"

#include "util/process.hpp"

#include <cstdlib>

namespace Toolchain {
namespace {

auto Environment(const char *name) -> std::filesystem::path {
  const auto *value = std::getenv(name);
  return value == nullptr ? std::filesystem::path{} : std::filesystem::path(value);
}

auto Emsdk() -> std::filesystem::path { return Root() / EmscriptenVersion / "emsdk"; }

} // namespace

auto Root() -> std::filesystem::path {
  if (const auto configured = Environment("SLAMHAUS_CACHE"); !configured.empty())
    return configured / "toolchains";
#ifdef _WIN32
  return Environment("LOCALAPPDATA") / "Slamhaus/toolchains";
#elif defined(__APPLE__)
  return Environment("HOME") / "Library/Caches/Slamhaus/toolchains";
#else
  if (const auto xdg = Environment("XDG_CACHE_HOME"); !xdg.empty())
    return xdg / "slamhaus/toolchains";
  return Environment("HOME") / ".cache/slamhaus/toolchains";
#endif
}

auto Compiler() -> std::filesystem::path {
#ifdef _WIN32
  const auto compiler = Emsdk() / "upstream/emscripten/em++.bat";
#else
  const auto compiler = Emsdk() / "upstream/emscripten/em++";
#endif
  return std::filesystem::is_regular_file(compiler) ? compiler
                                                    : std::filesystem::path{};
}

auto Status() -> nlohmann::json {
  const auto compiler = Compiler();
  return {{"name", "emscripten"},
          {"version", EmscriptenVersion},
          {"commit", EmsdkCommit},
          {"installed", !compiler.empty()},
          {"root", Emsdk().generic_string()},
          {"compiler", compiler.empty() ? nlohmann::json(nullptr)
                                        : nlohmann::json(compiler.string())}};
}

auto Install() -> std::string {
  const auto git = Process::FindExecutable("git");
  if (git.empty())
    return "Git is required to install the managed Emscripten toolchain.";
  const auto root = Emsdk();
  std::filesystem::create_directories(root.parent_path());
  if (!std::filesystem::exists(root)) {
    if (Process::Run(git, {"clone", "--filter=blob:none", "--no-checkout",
                           "https://github.com/emscripten-core/emsdk.git",
                           root.string()}) != 0)
      return "Could not download emsdk.";
  }
  if (Process::Run(git, {"-C", root.string(), "fetch", "--depth", "1", "origin",
                         EmsdkCommit}) != 0 ||
      Process::Run(git, {"-C", root.string(), "checkout", "--detach", EmsdkCommit}) !=
          0)
    return "Could not select the pinned emsdk revision.";
  auto python = Process::FindExecutable("python3");
  if (python.empty())
    python = Process::FindExecutable("python");
  if (python.empty())
    return "Python is required by emsdk.";
  const auto script = root / "emsdk.py";
  if (Process::Run(python, {script.string(), "install", EmscriptenVersion}) != 0)
    return "emsdk could not install Emscripten " + std::string(EmscriptenVersion) + ".";
  if (Process::Run(python, {script.string(), "activate", EmscriptenVersion}) != 0)
    return "emsdk installed but could not activate Emscripten.";
  return Compiler().empty() ? "Emscripten installation did not provide em++." : "";
}

auto Remove() -> std::string {
  std::error_code error;
  std::filesystem::remove_all(Root() / EmscriptenVersion, error);
  return error ? "Could not remove the managed Emscripten toolchain: " + error.message()
               : "";
}

} // namespace Toolchain
