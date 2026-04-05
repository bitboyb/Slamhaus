#include "section.hpp"
#include "../util/string.hpp"
#include "attributes.hpp"
#include "text.hpp"
#include <set>

namespace {
auto EmitAttributes(std::ostringstream &html,
                    const std::map<std::string, std::string> &attrs,
                    const std::set<std::string> &excluded = {}) -> void {
  for (const auto &[key, value] : attrs) {
    if (excluded.count(key) || value.empty()) {
      continue;
    }
    html << " " << key << "=\"" << value << "\"";
  }
}

auto MergeClass(const std::string &baseClass,
                const std::map<std::string, std::string> &attrs)
    -> std::string {
  if (!attrs.count("class") || attrs.at("class").empty()) {
    return baseClass;
  }
  if (baseClass.empty()) {
    return attrs.at("class");
  }
  return baseClass + " " + attrs.at("class");
}
} // namespace

namespace Section {
auto HandleHeroOpen(const std::string &line, std::ostringstream &html,
                    Parser::ParseState &pState) -> bool {
  std::string trimmed = String::Trim(line);
  if (trimmed.rfind(":hero[", 0) != 0) {
    return false;
  }

  Text::CloseLists(html, pState);

  auto attrs = Attributes::ParseAttributes(line, ":hero");
  std::string type = "standard";
  std::string media;

  if (attrs.count("type")) {
    type = attrs["type"];
  }
  if (attrs.count("media")) {
    media = attrs["media"];
  }

  if (type == "video") {
    html << "<section class=\"" << MergeClass("hero-video", attrs) << "\"";
    EmitAttributes(html, attrs, {"type", "media", "class"});
    html << ">\n";
    html << "<video src=\"" << media
         << "\" autoplay muted loop playsinline preload=\"auto\"></video>\n";
    html << "<div class=\"hero-content\">\n";
  } else if (type == "image") {
    html << "<section class=\"" << MergeClass("hero-image", attrs) << "\"";
    EmitAttributes(html, attrs, {"type", "media", "class", "style"});
    html << " style=\"background-image: url('" << media << "');";
    if (attrs.count("style") && !attrs.at("style").empty()) {
      html << " " << attrs.at("style");
    }
    html << "\">\n";
    html << "<div class=\"hero-content\">\n";
  } else {
    html << "<section class=\"" << MergeClass("hero", attrs) << "\"";
    EmitAttributes(html, attrs, {"type", "media", "class"});
    html << ">\n";
  }
  return true;
}

auto HandleHeroClose(const std::string &line, std::ostringstream &html)
    -> bool {
  if (String::Trim(line) != ":/hero") {
    return false;
  }

  html << "  </div>\n";
  html << "</section>\n";
  return true;
}

auto HandleSectionOpen(const std::string &line, std::ostringstream &html,
                       Parser::ParseState &pState) -> bool {
  std::string trimmed = String::Trim(line);
  if (trimmed.rfind(":section[", 0) != 0) {
    return false;
  }

  Text::CloseLists(html, pState);
  html << "<section";

  auto attrs = Attributes::ParseAttributes(line, ":section");
  EmitAttributes(html, attrs);
  html << ">\n";
  return true;
}

auto HandleSectionClose(const std::string &line, std::ostringstream &html)
    -> bool {
  if (String::Trim(line) != ":/section") {
    return false;
  }

  html << "</section>\n";
  return true;
}

auto HandleColumnOpen(const std::string &line, std::ostringstream &html,
                      Parser::ParseState &pState, Parser::ColumnState &cState)
    -> bool {
  std::string trimmed = String::Trim(line);
  if (trimmed.rfind(":column[", 0) != 0) {
    return false;
  }

  if (pState.inColumn && !cState.columnBuffer.str().empty()) {
    cState.columnParts.push_back(cState.columnBuffer.str());
    cState.columnOpenLines.push_back(line);
    cState.columnBuffer.str("");
    cState.columnBuffer.clear();
  } else if (!pState.inColumn) {
    Text::CloseLists(html, pState);
    pState.inColumn = true;
    cState.columnParts.clear();
    cState.columnOpenLines.clear();
    cState.columnBuffer.str("");
    cState.columnBuffer.clear();
    cState.columnOpenLines.push_back(line);
  }

  return true;
}

auto ApplyColumns(int fallbackNumber, const std::vector<std::string> &text,
                  const std::vector<std::string> &openLines) -> std::string {
  std::ostringstream oss;
  int numColumns = fallbackNumber;

  if (!openLines.empty()) {
    auto attrs = Attributes::ParseAttributes(openLines[0], ":column");
    if (attrs.count("size")) {
      try {
        numColumns = std::stoi(attrs["size"]);
      } catch (...) {
      }
    }
  }

  oss << "<div class=\"columns-container responsive-columns\">";
  for (int i = 0; i < numColumns; i++) {
    std::string content = i < (int)text.size() ? text[i] : "";
    std::string openLine = i < (int)openLines.size() ? openLines[i] : "";

    auto attrs = Attributes::ParseAttributes(openLine, ":column");
    oss << "<div class=\"" << MergeClass("column", attrs) << "\"";
    EmitAttributes(oss, attrs, {"size", "class"});
    oss << ">";
    oss << content << "</div>";
  }
  oss << "</div>";
  return oss.str();
}

auto HandleColumnClose(const std::string &line, std::ostringstream &html,
                       Parser::ParseState &pState, Parser::ColumnState &cState,
                       SEO::SEOData *seo) -> bool {
  if (String::Trim(line) != ":/column") {
    return false;
  }

  if (!cState.columnBuffer.str().empty()) {
    cState.columnParts.push_back(cState.columnBuffer.str());
  }
  for (auto &col : cState.columnParts) {
    col = Parser::ParseMarkdown(col, seo);
  }
  html << ApplyColumns(static_cast<int>(cState.columnParts.size()),
                       cState.columnParts, cState.columnOpenLines);
  pState.inColumn = false;
  cState.columnBuffer.str("");
  cState.columnBuffer.clear();
  return true;
}

auto HandleDivOpen(const std::string &line, std::ostringstream &html,
                   Parser::ParseState &pState) -> bool {
  std::string trimmed = String::Trim(line);
  if (trimmed.rfind(":div[", 0) != 0) {
    return false;
  }

  Text::CloseLists(html, pState);
  html << "<div";

  auto attrs = Attributes::ParseAttributes(trimmed, ":div");
  EmitAttributes(html, attrs);
  html << ">\n";
  return true;
}

auto HandleDivClose(const std::string &line, std::ostringstream &html) -> bool {
  if (String::Trim(line) != ":/div") {
    return false;
  }
  html << "</div>\n";
  return true;
}

auto HandleCanvasOpen(const std::string &line, std::ostringstream &html,
                      Parser::ParseState &pState) -> bool {
  std::string trimmed = String::Trim(line);
  if (trimmed.rfind(":canvas[", 0) != 0) {
    return false;
  }

  Text::CloseLists(html, pState);

  auto attrs = Attributes::ParseAttributes(trimmed, ":canvas");

  std::string id = attrs.count("id") ? attrs["id"] : "canvas";
  std::string width = attrs.count("width") ? attrs["width"] : "64";
  std::string height = attrs.count("height") ? attrs["height"] : "64";

  html << "<div class=\"canvas-wrapper\">\n";

  html << "<div class=\"canvas\" id=\"" << id << "\" data-width=\"" << width
       << "\" data-height=\"" << height << "\">\n";

  html << "<script type=\"module\">\n";
  html << "function fitCanvasSize_" << id << "() {\n";
  html << "  const canvas = document.getElementById('" << id << "');\n";
  html << "  const charsX = parseInt(canvas.dataset.width);\n";
  html << "  const charsY = parseInt(canvas.dataset.height);\n";
  html << "  canvas.style.width = `${charsX}ch`;\n";
  html << "  canvas.style.height = `${charsY}em`;\n";
  html << "}\n";
  html << "window.addEventListener('load', fitCanvasSize_" << id << ");\n";
  html << "window.addEventListener('resize', fitCanvasSize_" << id << ");\n";
  html << "</script>\n";

  return true;
}

auto HandleCanvasClose(const std::string &line, std::ostringstream &html)
    -> bool {
  if (String::Trim(line) != ":/canvas") {
    return false;
  }
  html << "</div>\n";
  html << "</div>\n";
  return true;
}

auto HandleBlockElements(const std::string &line, std::ostringstream &html,
                         Parser::ParseState &pState,
                         Parser::ColumnState &cState, SEO::SEOData *seo)
    -> bool {
  if (HandleHeroOpen(line, html, pState)) {
    return true;
  }
  if (HandleHeroClose(line, html)) {
    return true;
  }
  if (HandleSectionOpen(line, html, pState)) {
    return true;
  }
  if (HandleDivOpen(line, html, pState)) {
    return true;
  }
  if (HandleCanvasOpen(line, html, pState)) {
    return true;
  }
  if (HandleSectionClose(line, html)) {
    return true;
  }
  if (HandleColumnOpen(line, html, pState, cState)) {
    return true;
  }
  if (HandleColumnClose(line, html, pState, cState, seo)) {
    return true;
  }
  if (HandleDivClose(line, html)) {
    return true;
  }
  if (HandleCanvasClose(line, html)) {
    return true;
  }
  if (pState.inColumn) {
    if (line == "---") {
      cState.columnParts.push_back(cState.columnBuffer.str());
      cState.columnBuffer.str("");
      cState.columnBuffer.clear();
    } else {
      cState.columnBuffer << line << "\n";
    }
    return true;
  }
  return false;
}
} // namespace Section
