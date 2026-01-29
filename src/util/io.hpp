#pragma once

#ifndef IO_HPP
#define IO_HPP

#include <filesystem>
#include <string>

namespace Util {
/// Reads the entire content of a file into a string.
/// Returns an empty string if the file cannot be opened.
///
/// @param path The filesystem path to read.
/// @return File content or empty string.
auto ReadFile(const std::filesystem::path &path) -> std::string;
} // namespace Util

#endif // IO_HPP
