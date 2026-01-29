#pragma once

#ifndef STRING_HPP
#define STRING_HPP

#include <string>

namespace String {
/// Removes leading and trailing whitespace from the given string.
/// Returns an empty string if the input contains only whitespace.
///
/// @param str The input string.
/// @return A new string with whitespace trimmed.
auto Trim(const std::string &str) -> std::string;
} // namespace String

#endif // STRING_HPP
