#include "interaction.hpp"
#include "../util/string.hpp"
#include "attributes.hpp"
#include "parser.hpp"
#include "text.hpp"
#include <set>
#include <sstream>

namespace Interaction {
auto IsButtonLine(const std::string &line) -> bool {
  auto trimmed = String::Trim(line);
  return line.rfind("?button[", 0) == 0;
}

auto IsInputLine(const std::string &line) -> bool {
  auto trimmed = String::Trim(line);
  return line.rfind("?input[", 0) == 0;
}

auto IsTextareaLine(const std::string &line) -> bool {
  auto trimmed = String::Trim(line);
  return line.rfind("?textarea[", 0) == 0;
}

auto EmitAttributes(std::ostringstream &html,
                    const std::map<std::string, std::string> &attrs) -> void {
  static const std::set<std::string> boolAttrs = {
      "required", "checked", "disabled", "multiple", "autofocus"};

  for (auto const &kv : attrs) {
    auto const &key = kv.first;
    auto const &value = kv.second;

    if (boolAttrs.count(key)) {
      html << " " << key;
    } else if (!value.empty()) {
      html << " " << key << "=\"" << value << "\"";
    }
  }
}

auto ProcessButton(const std::string &line, Parser::ParseState &pState)
    -> std::string {
  auto trimmed = String::Trim(line);
  auto attrs = Attributes::ParseAttributes(trimmed, "?button");
  std::string action = attrs.count("action") ? attrs["action"] : "";
  std::string label = attrs.count("text") ? attrs["text"] : "Submit";
  std::string type =
      attrs.count("type") ? attrs["type"] : (action.empty() ? "submit" : "button");

  attrs.erase("action");
  attrs.erase("text");
  attrs["type"] = type;

  std::ostringstream html;
  html << "<button";

  if (!action.empty()) {
    if (action.rfind("mailto:", 0) == 0 || action.rfind("http://", 0) == 0 ||
        action.rfind("https://", 0) == 0 || action.rfind("/", 0) == 0 ||
        action.find(".html") != std::string::npos) {
      html << " onclick=\"window.location.href='" << action << "'\"";
    } else {
      html << " onclick=\"" << action << "()\"";
    }
  }

  EmitAttributes(html, attrs);
  html << ">" << label << "</button>";

  return html.str();
}

auto ProcessInput(const std::string &line, Parser::ParseState &pState)
    -> std::string {
  auto trimmed = String::Trim(line);
  auto attrs = Attributes::ParseAttributes(trimmed, "?input");
  std::ostringstream html;
  html << "<p><input";
  if (!pState.inForm) {
    html << " class=\"standalone-input\"";
  }
  EmitAttributes(html, attrs);
  html << " /></p>";
  return html.str();
}

auto ProcessTextarea(const std::string &line, Parser::ParseState &pState)
    -> std::string {
  auto trimmed = String::Trim(line);
  auto attrs = Attributes::ParseAttributes(trimmed, "?textarea");
  std::ostringstream html;
  html << "<p><textarea";
  EmitAttributes(html, attrs);
  html << "></textarea></p>";
  return html.str();
}

auto IsInteraction(const std::string &line, std::ostringstream &html,
                   Parser::ParseState &pState) -> bool {
  if (IsButtonLine(line)) {
    Text::CloseLists(html, pState);
    html << ProcessButton(line, pState) << "\n";
    return true;
  }
  if (IsInputLine(line)) {
    Text::CloseLists(html, pState);
    html << ProcessInput(line, pState) << "\n";
    return true;
  }
  if (IsTextareaLine(line)) {
    Text::CloseLists(html, pState);
    html << ProcessTextarea(line, pState) << "\n";
    return true;
  }

  return false;
}
} // namespace Interaction
