#pragma once

#include "language/language.hpp"

#include <filesystem>
#include <string_view>
#include <vector>

namespace Wasm {

auto DiscoverCompiler() -> std::filesystem::path;
auto Compile(const std::filesystem::path &scripts, const std::filesystem::path &output,
             const std::filesystem::path &compiler = {})
    -> std::vector<Language::Diagnostic>;
auto Runtime() -> std::string_view;
auto Sdk() -> std::string_view;

} // namespace Wasm
