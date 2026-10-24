#pragma once

#include <filesystem>
#include <string>

#include <nlohmann/json.hpp>

namespace Toolchain {

/** Pinned Emscripten release used for reproducible WebAssembly builds. */
inline constexpr auto EmscriptenVersion = "4.0.20";
/** Pinned emsdk commit corresponding to EmscriptenVersion. */
inline constexpr auto EmsdkCommit = "e4fe26ef59168ff44f4c23c466e497bf60b3411e";

/** Return the repository-local managed toolchain directory. */
auto Root() -> std::filesystem::path;
/** Return the pinned Emscripten compiler executable, if installed. */
auto Compiler() -> std::filesystem::path;
/** Return install and version state as JSON. */
auto Status() -> nlohmann::json;
/** Install the pinned toolchain and return an error string if it fails. */
auto Install() -> std::string;
/** Remove the managed toolchain and return an error string if it fails. */
auto Remove() -> std::string;

} // namespace Toolchain
