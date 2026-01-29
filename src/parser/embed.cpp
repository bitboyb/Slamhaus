#include "embed.hpp"
#include "attributes.hpp"
#include "parser.hpp"
#include "text.hpp"

#include <sstream>

namespace Embed {
static auto EscapeHtml(const std::string &input) -> std::string {
  std::string out;
  for (char c : input) {
    switch (c) {
    case '&':
      out += "&amp;";
      break;
    case '<':
      out += "&lt;";
      break;
    case '>':
      out += "&gt;";
      break;
    case '"':
      out += "&quot;";
      break;
    case '\'':
      out += "&#39;";
      break;
    default:
      out += c;
    }
  }
  return out;
}

static auto EmitAttrs(std::ostringstream &oss,
                      const std::map<std::string, std::string> &attrs) -> void {
  for (auto const &kv : attrs) {
    oss << " " << kv.first;
    if (!kv.second.empty()) {
      oss << "=\"" << EscapeHtml(kv.second) << "\"";
    }
  }
}

auto IsIFrameLine(const std::string &line) -> bool {
  return line.rfind("!iframe[", 0) == 0 &&
         line.find('(') != std::string::npos &&
         line.find(')') != std::string::npos;
}

auto IsImageLine(const std::string &line) -> bool {
  return line.rfind("![", 0) == 0 || line.rfind("!image[", 0) == 0;
}

auto IsVideoLine(const std::string &line) -> bool {
  return line.rfind("!video[", 0) == 0 || line.rfind("!videos[", 0) == 0;
}

auto IsAudioLine(const std::string &line) -> bool {
  return line.rfind("!audio[", 0) == 0;
}

auto IsPictureLine(const std::string &line) -> bool {
  return line.rfind("!picture[", 0) == 0 || line.rfind("!pictures[", 0) == 0;
}

auto IsSvgLine(const std::string &line) -> bool {
  return line.rfind("!svg[", 0) == 0;
}

auto ExtractContentAndStyle(const std::string &raw, std::string &content,
                            std::string &style) -> void {
  std::istringstream iss(raw);
  content.clear();
  style.clear();
  std::string token;
  while (iss >> token) {
    auto colon = token.find(':');
    if (colon != std::string::npos) {
      auto key = token.substr(0, colon);
      auto value = token.substr(colon + 1);
      if (key == "height" || key == "width") {
        style += key + ":" + (value == "max" ? "100%" : value) + "; ";
        continue;
      }
    }
    if (!content.empty()) {
      content += " ";
    }
    content += token;
  }
  if (content.empty()) {
    content = raw;
  }
}

auto ProcessImages(const std::string &line) -> std::string {
  auto open = line.find('[');
  auto close = line.find(']', open + 1);
  auto paren = line.find('(', close);
  auto end = line.find(')', paren + 1);

  std::string body = line.substr(open + 1, close - open - 1);
  std::string url = (paren != std::string::npos && end != std::string::npos)
                        ? line.substr(paren + 1, end - paren - 1)
                        : "";

  auto attrs = Attributes::ParseAttributes(line, "![");
  std::string alt;
  std::string styleFromTokens;
  ExtractContentAndStyle(body, alt, styleFromTokens);

  if (!styleFromTokens.empty()) {
    attrs["style"] += styleFromTokens;
  }
  if (!alt.empty() && !attrs.count("alt")) {
    attrs["alt"] = alt;
  }

  std::string link;

  if (attrs.count("link")) {
    link = attrs["link"];
    attrs.erase("link");
  } else if (attrs.count("href")) {
    link = attrs["href"];
    attrs.erase("href");
  }

  std::ostringstream oss;
  oss << "<img src=\"" << url << "\"";
  EmitAttrs(oss, attrs);
  oss << ">";

  if (!link.empty()) {
    return "<a href=\"" + link + "\">" + oss.str() + "</a>";
  }
  return oss.str();
}

auto ProcessIFrame(const std::string &line) -> std::string {
  auto open = line.find('[');
  auto close = line.find(']', open + 1);
  auto paren = line.find('(', close);
  auto end = line.find(')', paren + 1);

  std::string body = line.substr(open + 1, close - open - 1);
  std::string fallbackUrl =
      (paren != std::string::npos && end != std::string::npos)
          ? line.substr(paren + 1, end - paren - 1)
          : "";

  std::string dummy, styleFromTokens;
  ExtractContentAndStyle(body, dummy, styleFromTokens);

  auto attrs = Attributes::ParseAttributes(line, "!iframe");

  std::string url;
  if (attrs.count("link")) {
    url = attrs["link"];
    attrs.erase("link");
  } else if (attrs.count("href")) {
    url = attrs["href"];
    attrs.erase("href");
  } else {
    url = fallbackUrl;
  }

  if (!styleFromTokens.empty()) {
    attrs["style"] += styleFromTokens;
  }

  std::ostringstream oss;
  oss << "<div class=\"embed-container\">\n"
      << "<iframe src=\"" << url << "\"";
  EmitAttrs(oss, attrs);
  oss << " frameborder=\"0\" allowfullscreen "
         "referrerpolicy=\"strict-origin-when-cross-origin\"></iframe>\n"
         "</div>\n";

  return oss.str();
}

auto ProcessVideos(const std::string &line) -> std::string {
  std::string prefix =
      (line.rfind("!videos[", 0) == 0 ? "!videos[" : "!video[");
  auto open = line.find('[', prefix.size());
  auto close = line.find(']', open + 1);
  auto paren = line.find('(', close);
  auto end = line.find(')', paren + 1);

  std::string body = line.substr(open + 1, close - open - 1);
  std::string url = (paren != std::string::npos && end != std::string::npos)
                        ? line.substr(paren + 1, end - paren - 1)
                        : "";

  std::string desc, styleFromTokens;
  ExtractContentAndStyle(body, desc, styleFromTokens);

  auto attrs = Attributes::ParseAttributes(line, prefix);

  if (!styleFromTokens.empty()) {
    attrs["style"] += styleFromTokens;
  }

  if (!desc.empty() && !attrs.count("title")) {
    attrs["title"] = desc;
  }

  std::ostringstream oss;
  oss << "<video src=\"" << url << "\" controls";
  EmitAttrs(oss, attrs);
  oss << ">Your browser does not support the video tag.</video>";
  return oss.str();
}

auto ProcessAudio(const std::string &line) -> std::string {
  auto open = line.find('[');
  auto close = line.find(']', open + 1);
  auto paren = line.find('(', close);
  auto end = line.find(')', paren + 1);

  std::string body = line.substr(open + 1, close - open - 1);
  std::string url = (paren != std::string::npos && end != std::string::npos)
                        ? line.substr(paren + 1, end - paren - 1)
                        : "";

  std::string desc, styleFromTokens;
  ExtractContentAndStyle(body, desc, styleFromTokens);

  auto attrs = Attributes::ParseAttributes(line, "!audio");

  if (!styleFromTokens.empty()) {
    attrs["style"] += styleFromTokens;
  }

  if (!desc.empty() && !attrs.count("title")) {
    attrs["title"] = desc;
  }

  std::ostringstream oss;
  oss << "<audio src=\"" << url << "\" controls";
  EmitAttrs(oss, attrs);
  oss << ">Your browser does not support the audio element.</audio>";
  return oss.str();
}

auto ProcessPictures(const std::string &line) -> std::string {
  std::string img = ProcessImages(line);
  return "<picture>" + img + "</picture>";
}

auto ProcessSvg(const std::string &line) -> std::string {
  auto open = line.find('[');
  auto close = line.find(']', open + 1);
  auto paren = line.find('(', close);
  auto end = line.find(')', paren + 1);

  std::string body = line.substr(open + 1, close - open - 1);
  std::string url = (paren != std::string::npos && end != std::string::npos)
                        ? line.substr(paren + 1, end - paren - 1)
                        : "";

  std::string desc, styleFromTokens;
  ExtractContentAndStyle(body, desc, styleFromTokens);

  auto attrs = Attributes::ParseAttributes(line, "!svg");

  if (!styleFromTokens.empty()) {
    attrs["style"] += styleFromTokens;
  }

  if (!desc.empty() && !attrs.count("title")) {
    attrs["title"] = desc;
  }

  std::ostringstream oss;
  oss << "<object data=\"" << url << "\" type=\"image/svg+xml\"";
  EmitAttrs(oss, attrs);
  oss << ">Your browser does not support SVG.</object>";
  return oss.str();
}

auto HandleEmbeds(const std::string &line, std::ostringstream &html,
                  Parser::ParseState &pState) -> bool {
  Text::CloseLists(html, pState);

  if (IsIFrameLine(line)) {
    html << ProcessIFrame(line);
    return true;
  }
  if (IsAudioLine(line)) {
    html << ProcessAudio(line);
    return true;
  }
  if (IsPictureLine(line)) {
    html << ProcessPictures(line);
    return true;
  }
  if (IsSvgLine(line)) {
    html << ProcessSvg(line);
    return true;
  }
  if (IsImageLine(line)) {
    html << ProcessImages(line);
    return true;
  }
  if (IsVideoLine(line)) {
    html << ProcessVideos(line);
    return true;
  }
  return false;
}

} // namespace Embed
