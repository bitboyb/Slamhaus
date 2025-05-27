#pragma once

#ifndef SMD_HPP
#define SMD_HPP

#include <filesystem>

namespace SMD
{
    /// Returns true if the file has a supported Slamhaus Markdown extension.
    /// Supported formats include: `.md`, `.smd`, `.slam`
    ///
    /// @param path Filesystem path to check.
    /// @return True if the extension is a Slamhaus Markdown type.
    bool IsContentFile(const std::filesystem::path& path);
}

#endif // SMD_HPP
