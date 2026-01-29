#pragma once

#ifndef SCRIPT_HPP
#define SCRIPT_HPP

#include "../parser/parser.hpp"
#include <string>

namespace Script {
/// @param pState   Current parser state (used to close lists/paragraphs before
/// insertion).
/// @return True if the script was parsed and inserted; false otherwise.
auto HandleScript(const std::string &line, std::istringstream &iss,
                  std::ostringstream &html, Parser::ParseState &pState) -> bool;
} // namespace Script

#endif // SCRIPT_HPP
