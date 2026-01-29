#include "wasm.hpp"
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <vector>

namespace WASM {

bool IsSourceFile(const std::filesystem::path &path) {
  return path.extension() == ".cpp";
}

auto Compile(const std::vector<WASMFile> &files, const std::string &emccPath)
    -> bool {
  bool allSuccess = true;
  for (const auto &file : files) {
    std::filesystem::create_directories(
        std::filesystem::path(file.outputPath).parent_path());

    std::ostringstream cmd;
    cmd << emccPath << " \"" << file.inputPath << "\""
        << " -o \"" << file.outputPath << ".js\""
        << " -sEXPORT_ALL=1"
        << " -sEXPORTED_RUNTIME_METHODS=ccall,cwrap"
        << " -sMODULARIZE=1"
        << " -sEXPORT_ES6=1"
        << " -sENVIRONMENT=web"
        << " -std=c++17";

    std::cout << "Compiling WASM: " << file.inputPath << "\n";
    int result = std::system(cmd.str().c_str());
    if (result != 0) {
      std::cerr << "Failed to compile " << file.inputPath << "\n";
      allSuccess = false;
    }
  }
  return allSuccess;
}
} // namespace WASM
