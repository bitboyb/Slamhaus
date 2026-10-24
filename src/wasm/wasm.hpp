#pragma once

#include "language/language.hpp"

#include <filesystem>
#include <string_view>
#include <vector>

namespace Wasm {

/** Locate an available Emscripten compiler. */
auto DiscoverCompiler() -> std::filesystem::path;
/** Compile generated C++ sources and report source-mapped diagnostics. */
auto Compile(const std::filesystem::path &scripts, const std::filesystem::path &output,
             const std::filesystem::path &compiler = {})
    -> std::vector<Language::Diagnostic>;
/** Return the embedded browser runtime JavaScript. */
auto Runtime() -> std::string_view;
/** Return the embedded public SDK header used for page-local builds. */
auto Sdk() -> std::string_view;

} // namespace Wasm
