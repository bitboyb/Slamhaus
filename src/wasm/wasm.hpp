#pragma once

#ifndef WASM_HPP
#define WASM_HPP

#include <filesystem>
#include <string>
#include <vector>

namespace WASM {
/// Represents a `.cpp` source file and its intended WebAssembly output path.
/// The output path should omit the `.js` and `.wasm` extension.
struct WASMFile {
  std::string inputPath; ///< Path to the input `.cpp` file.
  std::string
      outputPath; ///< Path (excluding extension) for output `.js` and `.wasm`.
};

/// Compiles one or more C++ source files into WebAssembly modules using
/// Emscripten. Produces `.js` and `.wasm` files for each input using ES6 and
/// `MODULARIZE` mode.
///
/// @param files     A list of WASMFile structs representing files to compile.
/// @param emccPath  Optional path to the `emcc` compiler (default is `"emcc"`).
/// @return True if all files compiled successfully, false otherwise.
/// Checks if a file is a C++ source file suitable for WASM compilation.
bool IsSourceFile(const std::filesystem::path &path);

auto Compile(const std::vector<WASMFile> &files,
             const std::string &emccPath = "emcc") -> bool;
} // namespace WASM

#endif // WASM_HPP
