#pragma once

#ifndef ATTRIBUTES_HPP
#define ATTRIBUTES_HPP

#include <string>
#include <map>

namespace Attributes
{
    /// Parses key-value attributes from a Slamhaus-style declaration line.
    ///
    /// Accepts syntax like:
    ///     :section[align:"center" size:"3"](#id)
    ///
    /// - Extracts attributes like `align`, `size` as key-value pairs
    /// - Also captures optional ID tags in the form `(#some-id)`
    ///
    /// @param line   Full line of markup (e.g., :section[...]())
    /// @param prefix Expected start token (e.g., ":section")
    /// @return A map of parsed attributes including `"id"` if an ID tag is found
    std::map<std::string, std::string> ParseAttributes(const std::string& line,
                                                       const std::string& prefix);
}

#endif // ATTRIBUTES_HPP
