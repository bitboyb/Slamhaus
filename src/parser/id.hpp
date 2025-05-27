#pragma once

#ifndef ID_HPP
#define ID_HPP

#include <string>

namespace ID
{
    /// Extracts an inline ID tag from a line, e.g. "(#my-id)" becomes "my-id".
    /// Returns an empty string if no ID tag is found.
    ///
    /// @param line Line from markdown
    /// @return     Extracted ID string
    std::string ExtractIDTag(const std::string& line);

    /// Inserts an extracted ID tag into an HTML element string.
    /// Does nothing if an ID is already present.
    ///
    /// @param htmlLine      Existing HTML line (e.g. "<div class="x">")
    /// @param markdownLine  Original markdown line containing the ID tag
    /// @return              Modified HTML with ID injected, if needed
    std::string ApplyIDTag(const std::string& htmlLine, 
                           const std::string& markdownLine);

    /// Checks if the given markdown line contains an inline ID tag, e.g. "(#some-id)".
    ///
    /// @param line Markdown line
    /// @return     True if an ID tag is found
    bool isIDTag(const std::string& line);
}

#endif // ID_HPP
