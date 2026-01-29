#include "parser.hpp"
#include "../wasm/script.hpp"
#include "embed.hpp"
#include "form.hpp"
#include "interaction.hpp"
#include "section.hpp"
#include "seo.hpp"
#include "snippet.hpp"
#include "text.hpp"

#include <sstream>

namespace {
bool TryHandleBlock(const std::string &raw, std::istringstream &iss,
                    std::ostringstream &html, Parser::ParseState &pState,
                    Parser::ColumnState &cState, SEO::SEOData *seo) {
  if (raw.empty()) {
    return false;
  }
  if (raw[0] == '<') {
    Text::CloseLists(html, pState);
    html << raw << "\n";
    return true;
  }
  if (Script::HandleScript(raw, iss, html, pState)) {
    return true;
  }
  if (SEO::HandleMetaLines(raw, seo)) {
    return true;
  }
  if (Section::HandleBlockElements(raw, html, pState, cState, seo)) {
    return true;
  }
  if (Form::HandleFormElements(raw, html, pState)) {
    return true;
  }
  if (Interaction::IsInteraction(raw, html, pState)) {
    return true;
  }
  if (Embed::HandleEmbeds(raw, html, pState)) {
    return true;
  }
  if (Snippet::HandleCodeOrTable(raw, iss, html, pState)) {
    return true;
  }
  if (Text::HandleTextBlocks(raw, html, pState)) {
    return true;
  }
  return false;
}
} // namespace

namespace Parser {
auto ParseMarkdown(const std::string &markdown, SEO::SEOData *seo)
    -> std::string {
  std::istringstream iss(markdown);
  std::ostringstream html;
  std::string line;
  std::string buffer;

  Parser::ParseState pState;
  Parser::ColumnState cState;

  while (std::getline(iss, line)) {
    std::string trimmed = line;
    size_t first = trimmed.find_first_not_of(" \t");
    if (first == std::string::npos) {
      continue;
    }
    trimmed.erase(0, first);
    size_t last = trimmed.find_last_not_of(" \t");
    if (last != std::string::npos) {
      trimmed.erase(last + 1);
    }
    std::string raw;
    if (!pState.isBuffering) {
      if (!trimmed.empty() &&
          (trimmed[0] == ':' || trimmed[0] == '@' || trimmed[0] == '?' ||
           trimmed[0] == '$') &&
          trimmed.find('[') != std::string::npos &&
          trimmed.find("](") == std::string::npos) {
        pState.isBuffering = true;
        buffer = line;
        continue;
      } else {
        raw = line;
      }
    } else {
      buffer += "\n" + line;
      if (buffer.find("](") != std::string::npos) {
        pState.isBuffering = false;
        raw = buffer;
      } else {
        continue;
      }
    }

    TryHandleBlock(raw, iss, html, pState, cState, seo);
  }
  Text::CloseLists(html, pState);
  return html.str();
}
} // namespace Parser