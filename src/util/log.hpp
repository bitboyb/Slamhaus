#pragma once

#ifndef LOG_HPP
#define LOG_HPP

#include <string>

namespace Log {
/// Logs an informational message to stdout.
///
/// @param message The message to log.
auto Info(const std::string &message) -> void;

/// Logs an error message to stderr.
///
/// @param message The error message to log.
auto Error(const std::string &message) -> void;
} // namespace Log

#endif // LOG_HPP
