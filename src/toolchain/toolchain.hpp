#pragma once

#include <filesystem>
#include <string>

#include <nlohmann/json.hpp>

namespace Toolchain {

inline constexpr auto EmscriptenVersion = "4.0.20";
inline constexpr auto EmsdkCommit = "e4fe26ef59168ff44f4c23c466e497bf60b3411e";

auto Root() -> std::filesystem::path;
auto Compiler() -> std::filesystem::path;
auto Status() -> nlohmann::json;
auto Install() -> std::string;
auto Remove() -> std::string;

} // namespace Toolchain
