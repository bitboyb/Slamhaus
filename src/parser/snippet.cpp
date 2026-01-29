#include "text.hpp"

#include <iostream>
#include <regex>
#include <sstream>

namespace Snippet {
auto IsCodeBlockLine(const std::string &line) -> bool {
  return line.rfind("```", 0) == 0;
}

auto ReplaceInlineCode(const std::string &text) -> std::string {
  return std::regex_replace(text, std::regex(R"(`([^`]+)`)"),
                            "<code>$1</code>");
}

auto ParseCodeBlock(std::istream &iss, const std::string &openingLine)
    -> std::string {
  std::ostringstream codeBlock;
  std::string line;
  while (std::getline(iss, line)) {
    if (line.rfind("```", 0) == 0) {
      break;
    }
    codeBlock << line << "\n";
  }
  return "<pre><code>" + codeBlock.str() + "</code></pre>\n";
}

auto HandleCodeOrTable(const std::string &line, std::istringstream &iss,
                       std::ostringstream &html, Parser::ParseState &pState)
    -> bool {
  if (IsCodeBlockLine(line)) {
    if (!pState.inCodeBlock) {
      Text::CloseLists(html, pState);
      pState.inCodeBlock = true;
      html << ParseCodeBlock(iss, line);
      pState.inCodeBlock = false;
    }
    return true;
  }

  if (Text::IsTableLine(line)) {
    Text::CloseLists(html, pState);
    html << Text::ParseTable(iss, line);
    return true;
  }

  return false;
}
} // namespace Snippet