#pragma once

#ifndef EMBED_HPP
#define EMBED_HPP

#include <string>
#include <sstream>
#include "parser.hpp"

namespace Embed
{
    /// Parses and emits HTML for all Slamhaus embedded media types.
    /// Supported types:
    /// - `!image[]()` — standard image embeds with optional alt text, width, etc.
    /// - `!video[]()` — HTML5 video elements with style and controls
    /// - `!audio[]()` — HTML5 audio players with optional title
    /// - `!iframe[]()` — embeds external web content (e.g. YouTube)
    /// - `!svg[]()` — inline SVG content for crisp icons and illustrations
    /// - `!picture[]()` — wraps image in `<picture>` for future flexibility
    ///
    /// Automatically detects syntax and passes valid markup to `html`.
    /// Returns true if the line was handled as an embed.
    ///
    /// @param line     Line from markdown input
    /// @param html     Output HTML stream to append result to
    /// @param pState   Current parse state for managing block behavior
    bool HandleEmbeds(const std::string& line, 
                      std::ostringstream& html, 
                      Parser::ParseState& pState);
}

#endif // EMBED_HPP
