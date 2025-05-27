#pragma once

#ifndef CSS_HPP
#define CSS_HPP

#include <string>

namespace CSS
{
    /// Loads and concatenates all `.css` files in the specified directory.
    /// - Recursively traverses subdirectories
    /// - Skips unreadable files and logs failures to stderr
    /// - Adds comments with filenames between file contents
    ///
    /// @param cssPath Path to a directory containing CSS files
    /// @return A single string containing all CSS content
    std::string LoadCSS(const std::string& cssPath);
}

#endif // CSS_HPP
