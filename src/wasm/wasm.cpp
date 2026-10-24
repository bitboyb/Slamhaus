#include "wasm/wasm.hpp"

#include "toolchain/toolchain.hpp"
#include "util/process.hpp"
#include "wasm_runtime.hpp"
#include "wasm_sdk.hpp"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <map>

namespace Wasm {

auto Runtime() -> std::string_view { return RuntimeSource; }
auto Sdk() -> std::string_view { return SdkHeader; }

auto DiscoverCompiler() -> std::filesystem::path {
  if (const char *configured = std::getenv("SLAMHAUS_EMCXX")) {
    const std::filesystem::path path(configured);
    if (std::filesystem::exists(path))
      return path;
  }
  if (const auto managed = Toolchain::Compiler(); !managed.empty())
    return managed;
  if (const char *emsdk = std::getenv("EMSDK")) {
    const auto path = std::filesystem::path(emsdk) / "upstream/emscripten/em++";
    if (std::filesystem::exists(path))
      return path;
  }
  auto compiler = Process::FindExecutable("em++");
  return compiler.empty() ? Process::FindExecutable("emcc") : compiler;
}

auto Compile(const std::filesystem::path &scripts, const std::filesystem::path &output,
             const std::filesystem::path &requestedCompiler)
    -> std::vector<Language::Diagnostic> {
  std::vector<Language::Diagnostic> diagnostics;
  if (!std::filesystem::exists(scripts))
    return diagnostics;
  std::vector<std::filesystem::path> sources;
  for (const auto &entry : std::filesystem::recursive_directory_iterator(scripts)) {
    if (entry.is_regular_file() && entry.path().extension() == ".cpp") {
      sources.push_back(entry.path());
    }
  }
  if (sources.empty())
    return diagnostics;
  std::sort(sources.begin(), sources.end());
  const auto compiler =
      requestedCompiler.empty() ? DiscoverCompiler() : requestedCompiler;
  if (compiler.empty()) {
    diagnostics.push_back(
        {scripts,
         Language::Severity::Error,
         "SH3001",
         "C++ scripts require Emscripten, but em++/emcc was not found.",
         "Install Emscripten or set SLAMHAUS_EMCXX.",
         {}});
    return diagnostics;
  }
  const auto sdk = output / ".slamhaus-sdk/slam/slam.hpp";
  std::filesystem::create_directories(sdk.parent_path());
  std::ofstream(sdk, std::ios::binary) << SdkHeader;
  std::map<std::filesystem::path, std::vector<std::filesystem::path>> modules;
  for (const auto &source : sources) {
    const auto relative = std::filesystem::relative(source, scripts);
    auto destination = output / relative;
    if (relative.has_parent_path()) {
      destination = output / relative.parent_path();
      destination += ".js";
    } else {
      destination.replace_extension(".js");
    }
    modules[destination].push_back(source);
  }
  for (const auto &[destination, moduleSources] : modules) {
    std::filesystem::create_directories(destination.parent_path());
    std::vector<std::string> arguments;
    for (const auto &source : moduleSources)
      arguments.push_back(source.string());
    arguments.insert(arguments.end(),
                     {"-I" + scripts.string(),
                      "-I" + (output / ".slamhaus-sdk").string(),
                      "-I" + moduleSources.front().parent_path().string(), "-o",
                      destination.string(), "-std=c++17", "-Oz", "-flto", "-fno-rtti",
                      "-sFILESYSTEM=0", "-sASSERTIONS=0", "-sMALLOC=emmalloc",
                      "-sALLOW_MEMORY_GROWTH=1", "-sEXPORTED_RUNTIME_METHODS=ccall",
                      "-sMODULARIZE=1", "-sEXPORT_ES6=1", "-sENVIRONMENT=web"});
    if (Process::Run(compiler, arguments) != 0) {
      diagnostics.push_back({moduleSources.front(),
                             Language::Severity::Error,
                             "SH3002",
                             "Emscripten failed to compile module " +
                                 destination.filename().string() + ".",
                             {},
                             {}});
    }
  }
  std::error_code error;
  std::filesystem::remove_all(output / ".slamhaus-sdk", error);
  return diagnostics;
}

} // namespace Wasm
