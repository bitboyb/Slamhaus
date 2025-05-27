#pragma once

#ifndef SCRIPT_HPP
#define SCRIPT_HPP

#include <string>
#include <vector>
#include "../parser/parser.hpp"

namespace Script
{
    /// Handles a single `@script[...]()` block and emits the corresponding <script> glue code.
    /// Parses attributes like `src`, `call`, `args`, and `types`, and generates JavaScript that binds
    /// to WebAssembly-compiled C++ functions using Emscripten's `ccall` interface.
    ///
    /// @param line     The raw @script[...]() line.
    /// @param iss      Input stream for the rest of the Markdown file (for future expansion).
    /// @param html     Output stream for writing generated HTML/JS.
    /// @param pState   Current parser state (used to close lists/paragraphs before insertion).
    /// @return True if the script was parsed and inserted; false otherwise.
    bool HandleScript(const std::string& line,
                      std::istringstream& iss,
                      std::ostringstream& html,
                      Parser::ParseState& pState);
}

#endif // SCRIPT_HPP
