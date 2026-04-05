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

/// Checks if a file is a C++ source file suitable for WASM compilation.
///
/// @param path path to check
/// @return true if it is a source file
bool IsSourceFile(const std::filesystem::path &path);

/// Checks whether the configured Emscripten compiler is available.
///
/// @param emccPath Path or command name for `emcc`
/// @return True if the compiler can be invoked successfully
auto IsCompilerAvailable(const std::string &emccPath = "emcc") -> bool;

/// Compiles one or more C++ source files into WebAssembly modules using
/// Emscripten. Produces `.js` and `.wasm` files for each input using ES6 and
/// `MODULARIZE` mode.
///
/// @param files     A list of WASMFile structs representing files to compile.
/// @param emccPath  Optional path to the `emcc` compiler (default is `"emcc"`).
/// @return True if all files compiled successfully, false otherwise.
auto Compile(const std::vector<WASMFile> &files,
             const std::string &emccPath = "emcc") -> bool;
} // namespace WASM

#endif // WASM_HPP
