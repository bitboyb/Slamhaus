#pragma once

#ifndef LOG_HPP
#define LOG_HPP

#include <string>

namespace Log {
/// Logs an informational message to stdout.
///
/// @param message The message to log.
void Info(const std::string &message);

/// Logs an error message to stderr.
///
/// @param message The error message to log.
void Error(const std::string &message);
} // namespace Log

#endif // LOG_HPP
