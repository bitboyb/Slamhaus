#pragma once

#ifndef CLI_HPP
#define CLI_HPP

#include <string>

namespace CLI {
/// Returns the Slamhaus ASCII logo string.
std::string GetSlamhausLogo();

/// Returns version, author, and product summary text.
std::string GetInfo();

/// Returns the command-line usage text for Slamhaus.
std::string GetUsage();
} // namespace CLI

#endif // CLI_HPP
