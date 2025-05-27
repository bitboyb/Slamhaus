#pragma once

#ifndef STRING_HPP
#define STRING_HPP

#include <string>

namespace String
{
    /// Removes leading and trailing whitespace from the given string.
    /// Returns an empty string if the input contains only whitespace.
    ///
    /// @param str The input string.
    /// @return A new string with whitespace trimmed.
    std::string Trim(const std::string& str);
}

#endif // STRING_HPP
