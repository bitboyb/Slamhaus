#include "../util/html.hpp"
#include "parser.hpp"
#include "snippet.hpp"
#include <cctype>
#include <iostream>
#include <regex>
#include <sstream>
#include <vector>

namespace Text {
auto CloseParagraphs(std::ostringstream &html) -> void {
  std::string current = html.str();
  if (current.size() >= 3 && current.substr(current.size() - 3) == "<p>") {
    html.seekp(-3, std::ios_base::end);
    html << "";
  } else if (current.size() >= 4 &&
             current.substr(current.size() - 4) == "<p>\n") {
    html.seekp(-4, std::ios_base::end);
    html << "";
  }
}

auto ReplaceBoldItalic(const std::string &text) -> std::string {
  return std::regex_replace(text, std::regex(R"(\*\*\*(.+?)\*\*\*)"),
                            "<strong><em>$1</em></strong>");
}

auto ReplaceBold(const std::string &text) -> std::string {
  return std::regex_replace(text, std::regex(R"(\*\*(.+?)\*\*)"),
                            "<strong>$1</strong>");
}

auto ReplaceItalic(const std::string &text) -> std::string {
  return std::regex_replace(text, std::regex(R"(\*(.+?)\*)"), "<em>$1</em>");
}

auto ProcessLinks(const std::string &input) -> std::string {
  std::string out = input;
  size_t pos = 0;
  while ((pos = out.find("[", pos)) != std::string::npos) {
    size_t endText = out.find("]", pos + 1);
    if ((endText != std::string::npos) && ((endText + 1) < out.size()) &&
        (out[endText + 1] == '(')) {
      size_t endUrl = out.find(")", endText + 2);
      if (endUrl != std::string::npos) {
        std::string linkText = out.substr(pos + 1, endText - pos - 1);
        std::string url = out.substr(endText + 2, endUrl - (endText + 2));
        url = HTML::AdjustLinks(url);
        std::string linkTag = "<a href=\"" + url + "\">" + linkText + "</a>";
        out.replace(pos, endUrl - pos + 1, linkTag);
        pos += linkTag.size();
      } else {
        pos = endText + 1;
      }
    } else {
      pos++;
    }
  }

  return out;
}

auto ProcessInline(const std::string &text) -> std::string {
  std::string out = text;
  out = ProcessLinks(out);
  out = Snippet::ReplaceInlineCode(out);
  out = ReplaceBoldItalic(out);
  out = ReplaceBold(out);
  out = ReplaceItalic(out);
  return out;
}

auto IsNumberedListLine(const std::string &line) -> bool {
  size_t pos = 0;
  while ((pos < line.size()) && std::isdigit(line[pos])) {
    pos++;
  }
  return ((pos > 0) && (pos + 1 < line.size()) && (line[pos] == '.') &&
          (line[pos + 1] == ' '));
}

auto IsUnorderedListLine(const std::string &line) -> bool {
  return line.find("- ") == 0;
}

auto IsHorizontalRuleLine(const std::string &line) -> bool {
  return std::regex_match(line, std::regex(R"(^-{3,}\s*$)")) || (line == "---");
}

auto IsTableLine(const std::string &line) -> bool {
  return !line.empty() && line[0] == '|';
}

auto IsHeading1(const std::string &line) -> bool {
  return line.find("# ") == 0;
}

auto IsHeading2(const std::string &line) -> bool {
  return line.find("## ") == 0;
}

auto IsHeading3(const std::string &line) -> bool {
  return line.find("### ") == 0;
}

auto IsHeading4(const std::string &line) -> bool {
  return line.find("#### ") == 0;
}

auto AppendHeading(std::ostringstream &html, int level, const std::string &line)
    -> void {
  html << "<h" << level << ">" << ProcessInline(line) << "</h" << level
       << ">\n";
}

auto AppendParagraph(std::ostringstream &html, const std::string &line)
    -> void {
  html << "<p>" << ProcessInline(line) << "</p>\n";
}

auto AppendListItem(std::ostringstream &html, const std::string &line) -> void {
  html << "<li>" << ProcessInline(line) << "</li>\n";
}

auto CloseLists(std::ostringstream &html, Parser::ParseState &pState) -> void {
  if (pState.inUl) {
    html << "</ul>\n";
    pState.inUl = false;
  }
  if (pState.inOl) {
    html << "</ol>\n";
    pState.inOl = false;
  }
}

auto ParseTable(std::istream &iss, const std::string &firstLine)
    -> std::string {
  std::vector<std::string> tableLines;
  tableLines.push_back(firstLine);
  std::string next;
  while (std::getline(iss, next)) {
    if ((!next.empty()) && (next[0] == '|')) {
      tableLines.push_back(next);
    } else {
      break;
    }
  }
  if (tableLines.empty()) {
    std::cerr << "Error: Table has no content." << std::endl;
    return "";
  }
  std::ostringstream tableHtml;
  tableHtml << "<table>\n";
  bool headerFound = false;

  if (tableLines.size() >= 2) {
    std::regex sepRegex(R"(^\|[\-\:\s\|]+$)");

    if (std::regex_match(tableLines[1], sepRegex)) {
      headerFound = true;
    }
  }
  for (size_t i = 0; i < tableLines.size(); i++) {
    if (headerFound && (i == 1)) {
      continue;
    }
    tableHtml << "<tr>";
    std::istringstream rowStream(tableLines[i]);
    std::string cell;
    if ((!tableLines[i].empty()) && (tableLines[i][0] == '|')) {
      std::getline(rowStream, cell, '|');
    }
    while (std::getline(rowStream, cell, '|')) {
      cell = std::regex_replace(cell, std::regex(R"(^\s+|\s+$)"), "");
      if (headerFound && (i == 0)) {
        tableHtml << "<th>" << cell << "</th>";
      } else {
        tableHtml << "<td>" << cell << "</td>";
      }
    }
    tableHtml << "</tr>\n";
  }
  tableHtml << "</table>\n";
  return tableHtml.str();
}

auto HandleTextBlocks(const std::string &line, std::ostringstream &html,
                      Parser::ParseState &pState) -> bool {
  if (line.empty()) {
    CloseLists(html, pState);
    return true;
  }
  if (IsHorizontalRuleLine(line)) {
    CloseLists(html, pState);
    html << "<hr>\n";
    return true;
  }
  if (IsHeading1(line)) {
    CloseLists(html, pState);
    AppendHeading(html, 1, line.substr(2));
    return true;
  }
  if (IsHeading2(line)) {
    CloseLists(html, pState);
    AppendHeading(html, 2, line.substr(3));
    return true;
  }
  if (IsHeading3(line)) {
    CloseLists(html, pState);
    AppendHeading(html, 3, line.substr(4));
    return true;
  }
  if (IsHeading4(line)) {
    CloseLists(html, pState);
    AppendHeading(html, 4, line.substr(5));
    return true;
  }
  if (IsUnorderedListLine(line)) {
    if (!pState.inUl) {
      CloseLists(html, pState);
      html << "<ul>\n";
      pState.inUl = true;
    }
    AppendListItem(html, line.substr(2));
    return true;
  }
  if (IsNumberedListLine(line)) {
    if (!pState.inOl) {
      CloseLists(html, pState);
      html << "<ol>\n";
      pState.inOl = true;
    }
    size_t pos = line.find(". ");
    if (pos != std::string::npos) {
      AppendListItem(html, line.substr(pos + 2));
    } else {
      AppendListItem(html, line);
    }
    return true;
  }
  CloseLists(html, pState);
  AppendParagraph(html, line);
  return true;
}
} // namespace Text