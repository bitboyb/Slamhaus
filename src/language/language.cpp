#include "language/language.hpp"

#include "syntax/syntax.hpp"
#include "version.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <optional>
#include <set>
#include <sstream>
#include <unordered_map>
#include <utility>

#include <cmark-gfm-core-extensions.h>
#include <cmark-gfm-extension_api.h>
#include <cmark-gfm.h>
#include <yaml-cpp/yaml.h>

namespace Language {
namespace {

struct Line {
  std::string text;
  std::size_t number = 0;
};

struct ComponentSpec {
  const char *name;
  const char *tag;
  std::vector<const char *> positional;
  std::set<std::string> properties;
  bool container = false;
  const char *defaultEvent = nullptr;
};

auto ComponentSpecs() -> const std::vector<ComponentSpec> & {
  static const std::vector<ComponentSpec> specs = {
      {"Row", "div", {}, {"class", "gap"}, true},
      {"Column", "div", {}, {"class", "gap"}, true},
      {"Panel", "section", {}, {"class", "label"}, true},
      {"Form", "form", {}, {"class", "action", "method"}, true, "Submit"},
      {"Button", "button", {"label"}, {"class", "label", "type"}, false, "Click"},
      {"TextInput",
       "input",
       {"label"},
       {"class", "label", "value", "placeholder", "required", "disabled", "name"},
       false,
       "Change"},
      {"Number",
       "input",
       {"label"},
       {"class", "label", "value", "min", "max", "step", "required", "disabled",
        "name"},
       false,
       "Change"},
      {"Range",
       "input",
       {"label"},
       {"class", "label", "value", "min", "max", "step", "disabled", "name"},
       false,
       "Input"},
      {"Checkbox",
       "input",
       {"label"},
       {"class", "label", "checked", "disabled", "name"},
       false,
       "Change"},
      {"Select",
       "select",
       {"label"},
       {"class", "label", "options", "value", "required", "disabled", "name"},
       false,
       "Change"},
      {"TextArea",
       "textarea",
       {"label"},
       {"class", "label", "value", "placeholder", "rows", "required", "disabled",
        "name"},
       false,
       "Change"},
      {"Text", "span", {"text"}, {"class", "text"}},
      {"Output", "output", {"text"}, {"class", "text", "live"}},
      {"Image", "img", {}, {"class", "src", "alt", "width", "height"}},
      {"Audio", "audio", {}, {"class", "src", "controls", "loop", "muted", "autoplay"}},
      {"Video",
       "video",
       {},
       {"class", "src", "controls", "loop", "muted", "autoplay", "playsinline", "width",
        "height"}},
      {"Canvas", "canvas", {}, {"class", "width", "height", "responsive"}},
      {"Include", "include", {"name"}, {"name"}},
      {"Element", "div", {"tag"}, {"tag", "class", "label"}, true}};
  return specs;
}

auto FindSpec(const std::string &name) -> const ComponentSpec * {
  const auto &specs = ComponentSpecs();
  const auto found = std::find_if(specs.begin(), specs.end(),
                                  [&](const auto &spec) { return name == spec.name; });
  return found == specs.end() ? nullptr : &*found;
}

auto Trim(const std::string &value) -> std::string {
  const auto first = std::find_if_not(value.begin(), value.end(), [](char character) {
    return std::isspace(static_cast<unsigned char>(character)) != 0;
  });
  const auto last = std::find_if_not(value.rbegin(), value.rend(), [](char character) {
                      return std::isspace(static_cast<unsigned char>(character)) != 0;
                    }).base();
  return first < last ? std::string(first, last) : std::string{};
}

auto StartsWith(const std::string &value, const std::string &prefix) -> bool {
  return value.compare(0, prefix.size(), prefix) == 0;
}

auto Lower(std::string value) -> std::string {
  std::transform(value.begin(), value.end(), value.begin(), [](char character) {
    return static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
  });
  return value;
}

auto Utf16Length(const std::string &value) -> std::size_t {
  std::size_t units = 0;
  for (std::size_t i = 0; i < value.size();) {
    const auto byte = static_cast<unsigned char>(value[i]);
    std::size_t width = 1;
    if ((byte & 0x80U) == 0U)
      width = 1;
    else if ((byte & 0xE0U) == 0xC0U)
      width = 2;
    else if ((byte & 0xF0U) == 0xE0U)
      width = 3;
    else if ((byte & 0xF8U) == 0xF0U)
      width = 4;
    if (i + width > value.size())
      width = 1;
    units += width == 4 ? 2U : 1U;
    i += width;
  }
  return units;
}

auto SplitLines(const std::string &source, std::size_t first = 0) -> std::vector<Line> {
  std::vector<Line> lines;
  std::istringstream input(source);
  std::string line;
  while (std::getline(input, line)) {
    if (!line.empty() && line.back() == '\r')
      line.pop_back();
    lines.push_back({line, first++});
  }
  return lines;
}

auto SeverityName(Severity severity) -> const char * {
  return severity == Severity::Error ? "error" : "warning";
}

auto SafeId(const std::string &name) -> std::string {
  std::string value = "slam-";
  for (char character : name) {
    if (std::isalnum(static_cast<unsigned char>(character)) != 0)
      value.push_back(
          static_cast<char>(std::tolower(static_cast<unsigned char>(character))));
    else
      value.push_back('-');
  }
  return value;
}

enum class TokenKind : std::uint8_t {
  Identifier,
  String,
  Number,
  Boolean,
  LeftParen,
  RightParen,
  LeftBrace,
  RightBrace,
  LeftBracket,
  RightBracket,
  Comma,
  Colon,
  Equal,
  Semicolon,
  Arrow,
  Bind,
  End,
  Invalid
};

struct Token {
  TokenKind kind = TokenKind::Invalid;
  std::string text;
  Range range;
};

class Lexer {
public:
  Lexer(std::string source, std::size_t firstLine)
      : source_(std::move(source)), line_(firstLine) {}

  auto Run() -> std::vector<Token> {
    std::vector<Token> tokens;
    while (true) {
      SkipTrivia();
      const auto start = Position{line_, column_};
      if (position_ >= source_.size()) {
        tokens.push_back({TokenKind::End, {}, {start, start}});
        return tokens;
      }
      const char character = source_[position_];
      if (std::isalpha(static_cast<unsigned char>(character)) != 0 ||
          character == '_') {
        tokens.push_back(Identifier());
      } else if (std::isdigit(static_cast<unsigned char>(character)) != 0 ||
                 (character == '-' && position_ + 1 < source_.size() &&
                  std::isdigit(static_cast<unsigned char>(source_[position_ + 1])) !=
                      0)) {
        tokens.push_back(Number());
      } else if (character == '"') {
        tokens.push_back(String());
      } else if (character == '=' && Peek(1) == '>') {
        Advance();
        Advance();
        tokens.push_back({TokenKind::Bind, "=>", {start, {line_, column_}}});
      } else if (character == '-' && Peek(1) == '>') {
        Advance();
        Advance();
        tokens.push_back({TokenKind::Arrow, "->", {start, {line_, column_}}});
      } else {
        const auto kind = Punctuation(character);
        Advance();
        tokens.push_back({kind, std::string(1, character), {start, {line_, column_}}});
      }
    }
  }

private:
  auto Peek(std::size_t offset) const -> char {
    return position_ + offset < source_.size() ? source_[position_ + offset] : '\0';
  }

  auto Advance() -> void {
    if (position_ >= source_.size())
      return;
    if (source_[position_] == '\n') {
      ++line_;
      column_ = 0;
    } else {
      ++column_;
    }
    ++position_;
  }

  auto SkipTrivia() -> void {
    while (position_ < source_.size()) {
      if (std::isspace(static_cast<unsigned char>(source_[position_])) != 0) {
        Advance();
      } else if (Peek(0) == '/' && Peek(1) == '/') {
        while (position_ < source_.size() && Peek(0) != '\n')
          Advance();
      } else if (Peek(0) == '/' && Peek(1) == '*') {
        Advance();
        Advance();
        while (position_ < source_.size() && !(Peek(0) == '*' && Peek(1) == '/'))
          Advance();
        if (position_ < source_.size()) {
          Advance();
          Advance();
        }
      } else {
        return;
      }
    }
  }

  auto Identifier() -> Token {
    const auto start = Position{line_, column_};
    const auto begin = position_;
    while (std::isalnum(static_cast<unsigned char>(Peek(0))) != 0 || Peek(0) == '_')
      Advance();
    auto text = source_.substr(begin, position_ - begin);
    const auto kind =
        text == "true" || text == "false" ? TokenKind::Boolean : TokenKind::Identifier;
    return {kind, std::move(text), {start, {line_, column_}}};
  }

  auto Number() -> Token {
    const auto start = Position{line_, column_};
    const auto begin = position_;
    if (Peek(0) == '-')
      Advance();
    while (std::isdigit(static_cast<unsigned char>(Peek(0))) != 0)
      Advance();
    if (Peek(0) == '.') {
      Advance();
      while (std::isdigit(static_cast<unsigned char>(Peek(0))) != 0)
        Advance();
    }
    return {TokenKind::Number,
            source_.substr(begin, position_ - begin),
            {start, {line_, column_}}};
  }

  auto String() -> Token {
    const auto start = Position{line_, column_};
    Advance();
    std::string value;
    bool escaped = false;
    while (position_ < source_.size()) {
      const char character = Peek(0);
      Advance();
      if (escaped) {
        value.push_back(character == 'n' ? '\n' : character);
        escaped = false;
      } else if (character == '\\') {
        escaped = true;
      } else if (character == '"') {
        return {TokenKind::String, std::move(value), {start, {line_, column_}}};
      } else {
        value.push_back(character);
      }
    }
    return {TokenKind::Invalid, std::move(value), {start, {line_, column_}}};
  }

  static auto Punctuation(char character) -> TokenKind {
    switch (character) {
    case '(':
      return TokenKind::LeftParen;
    case ')':
      return TokenKind::RightParen;
    case '{':
      return TokenKind::LeftBrace;
    case '}':
      return TokenKind::RightBrace;
    case '[':
      return TokenKind::LeftBracket;
    case ']':
      return TokenKind::RightBracket;
    case ',':
      return TokenKind::Comma;
    case ':':
      return TokenKind::Colon;
    case '=':
      return TokenKind::Equal;
    case ';':
      return TokenKind::Semicolon;
    default:
      return TokenKind::Invalid;
    }
  }

  std::string source_;
  std::size_t position_ = 0;
  std::size_t line_ = 0;
  std::size_t column_ = 0;
};

class UiParser {
public:
  UiParser(std::string source, std::filesystem::path path, std::size_t firstLine)
      : tokens_(Lexer(std::move(source), firstLine).Run()), path_(std::move(path)) {}

  auto Run() -> std::pair<std::vector<Node>, std::vector<Diagnostic>> {
    auto nodes = ParseNodes(false);
    Validate(nodes);
    return {std::move(nodes), std::move(diagnostics_)};
  }

private:
  auto Current() const -> const Token & { return tokens_[position_]; }
  auto Is(TokenKind kind) const -> bool { return Current().kind == kind; }
  auto Take() -> Token { return tokens_[position_++]; }

  auto Error(const Token &token, std::string code, std::string message,
             std::string help = {}) -> void {
    diagnostics_.push_back({path_, Severity::Error, std::move(code), std::move(message),
                            std::move(help), token.range});
  }

  auto Expect(TokenKind kind, const std::string &message) -> std::optional<Token> {
    if (!Is(kind)) {
      Error(Current(), "SH1101", message);
      return std::nullopt;
    }
    return Take();
  }

  auto ParseNodes(bool nested) -> std::vector<Node> {
    std::vector<Node> nodes;
    while (!Is(TokenKind::End) && !(nested && Is(TokenKind::RightBrace))) {
      if (!Is(TokenKind::Identifier)) {
        Error(Take(), "SH1101", "Expected a Slam UI component name.");
        Synchronize();
        continue;
      }
      auto node = ParseNode();
      if (node.has_value())
        nodes.push_back(std::move(*node));
    }
    if (nested) {
      if (Is(TokenKind::RightBrace))
        Take();
      else
        Error(Current(), "SH1102", "Component block is not closed.");
    }
    return nodes;
  }

  auto ParseNode() -> std::optional<Node> {
    const auto type = Take();
    Node node;
    node.kind = NodeKind::Component;
    node.name = type.text;
    node.range.start = type.range.start;
    const auto *spec = FindSpec(node.name);
    if (spec == nullptr)
      Error(type, "SH1103", "Unknown Slam UI component " + node.name + ".");

    if (Is(TokenKind::Identifier) &&
        tokens_[position_ + 1].kind == TokenKind::LeftParen)
      node.id = Take().text;
    if (!Expect(TokenKind::LeftParen, "Expected '(' after component name.")) {
      Synchronize();
      return node;
    }
    ParseArguments(node, spec);
    if (!Expect(TokenKind::RightParen, "Expected ')' after component arguments.")) {
      Synchronize();
      return node;
    }

    const auto hasBlock = Is(TokenKind::LeftBrace);
    if (hasBlock)
      ParseBlock(node, spec, type);

    if (Is(TokenKind::Identifier) && Current().text == "on") {
      Take();
      if (Is(TokenKind::Identifier))
        node.binding.event = Take().text;
      else
        Error(Current(), "SH1105", "Expected an event name after 'on'.");
    }
    if (Is(TokenKind::Bind))
      ParseBinding(node, spec);
    if (hasBlock) {
      if (Is(TokenKind::Semicolon))
        Take();
    } else if (!Expect(TokenKind::Semicolon, "Expected ';' after component declaration.")) {
      Synchronize();
    }
    node.range.end = tokens_[position_ == 0 ? 0 : position_ - 1].range.end;
    return node;
  }

  auto ParseArguments(Node &node, const ComponentSpec *spec) -> void {
    std::size_t positional = 0;
    while (!Is(TokenKind::RightParen) && !Is(TokenKind::End)) {
      if (Is(TokenKind::Identifier) &&
          tokens_[position_ + 1].kind == TokenKind::Colon) {
        const auto key = Take();
        Take();
        const auto value = ParseValue();
        if (value.has_value())
          node.properties[key.text] = *value;
        if (spec != nullptr && spec->properties.count(key.text) == 0U)
          Error(key, "SH1106",
                "Unknown property " + key.text + " on " + node.name + ".");
      } else {
        const auto value = ParseValue();
        if (value.has_value()) {
          if (spec == nullptr || positional >= spec->positional.size())
            Error(tokens_[position_ - 1], "SH1107",
                  "Too many positional arguments for " + node.name + ".");
          else
            node.properties[spec->positional[positional++]] = *value;
        }
      }
      if (Is(TokenKind::Comma))
        Take();
      else if (!Is(TokenKind::RightParen)) {
        Error(Current(), "SH1108", "Expected ',' between component arguments.");
        break;
      }
    }
  }

  auto ParseBlock(Node &node, const ComponentSpec *spec, const Token &type) -> void {
    Take();
    while (!Is(TokenKind::RightBrace) && !Is(TokenKind::End)) {
      if (Is(TokenKind::Identifier) && Current().text == "on") {
        Take();
        if (Is(TokenKind::Identifier))
          node.binding.event = Take().text;
        else
          Error(Current(), "SH1105", "Expected an event name after 'on'.");
        ParseBinding(node, spec);
        Expect(TokenKind::Semicolon, "Expected ';' after event binding.");
        continue;
      }
      if (Is(TokenKind::Identifier) && tokens_[position_ + 1].kind == TokenKind::Equal) {
        const auto property = Take();
        Take();
        const auto value = ParseValue();
        if (value.has_value())
          node.properties[property.text] = *value;
        if (spec != nullptr && spec->properties.count(property.text) == 0U)
          Error(property, "SH1106", "Unknown property " + property.text + " on " +
                                      node.name + ".");
        Expect(TokenKind::Semicolon, "Expected ';' after property assignment.");
        continue;
      }
      if (!Is(TokenKind::Identifier)) {
        Error(Take(), "SH1101", "Expected a property assignment or child component.");
        Synchronize();
        continue;
      }
      auto child = ParseNode();
      if (child.has_value())
        node.children.push_back(std::move(*child));
    }
    if (!Expect(TokenKind::RightBrace, "Component block is not closed.") &&
        spec != nullptr && !spec->container)
      Error(type, "SH1104", node.name + " cannot contain child components.");
    if (spec != nullptr && !spec->container && !node.children.empty())
      Error(type, "SH1104", node.name + " cannot contain child components.");
  }

  auto ParseValue() -> std::optional<std::string> {
    if (Is(TokenKind::String) || Is(TokenKind::Number) || Is(TokenKind::Boolean) ||
        Is(TokenKind::Identifier))
      return Take().text;
    if (Is(TokenKind::LeftBracket)) {
      Take();
      std::ostringstream values;
      bool first = true;
      while (!Is(TokenKind::RightBracket) && !Is(TokenKind::End)) {
        if (!(Is(TokenKind::String) || Is(TokenKind::Number) ||
              Is(TokenKind::Boolean))) {
          Error(Current(), "SH1109",
                "List values must be strings, numbers, or booleans.");
          return std::nullopt;
        }
        if (!first)
          values << '\n';
        values << Take().text;
        first = false;
        if (Is(TokenKind::Comma))
          Take();
        else if (!Is(TokenKind::RightBracket))
          Error(Current(), "SH1108", "Expected ',' between list values.");
      }
      Expect(TokenKind::RightBracket, "Expected ']' after list.");
      return values.str();
    }
    Error(Current(), "SH1109", "Expected a property value.");
    return std::nullopt;
  }

  auto ParseBinding(Node &node, const ComponentSpec *spec) -> void {
    Take();
    if (node.binding.event.empty() && spec != nullptr && spec->defaultEvent != nullptr)
      node.binding.event = spec->defaultEvent;
    if (!Is(TokenKind::Identifier)) {
      Error(Current(), "SH1110", "Expected a C++ function after '=>'.");
      return;
    }
    node.binding.function = Take().text;
    if (!Expect(TokenKind::LeftParen, "Expected '(' after function name."))
      return;
    while (!Is(TokenKind::RightParen) && !Is(TokenKind::End)) {
      if (Is(TokenKind::Identifier)) {
        node.binding.arguments.push_back(Take().text);
      } else if (Is(TokenKind::String) || Is(TokenKind::Number) ||
                 Is(TokenKind::Boolean)) {
        node.binding.arguments.push_back("$literal$" + Take().text);
      } else
        Error(Current(), "SH1111",
              "Binding arguments must be component names or literals.");
      if (Is(TokenKind::Comma))
        Take();
      else if (!Is(TokenKind::RightParen))
        Error(Current(), "SH1108", "Expected ',' between binding arguments.");
    }
    Expect(TokenKind::RightParen, "Expected ')' after binding arguments.");
    if (Is(TokenKind::Arrow)) {
      Take();
      if (Is(TokenKind::Identifier))
        node.binding.output = Take().text;
      else
        Error(Current(), "SH1112", "Expected an output component after '->'.");
    }
    if (node.binding.event.empty())
      Error(tokens_[position_ - 1], "SH1113",
            node.name + " has no default event; specify 'on Event'.");
  }

  auto Synchronize() -> void {
    while (!Is(TokenKind::End) && !Is(TokenKind::Semicolon) &&
           !Is(TokenKind::RightBrace))
      Take();
    if (Is(TokenKind::Semicolon))
      Take();
  }

  auto Validate(const std::vector<Node> &nodes) -> void {
    std::map<std::string, const Node *> symbols;
    const auto collect = [&](const auto &self,
                             const std::vector<Node> &values) -> void {
      for (const auto &node : values) {
        if (!node.binding.function.empty() && node.id.empty())
          diagnostics_.push_back({path_,
                                  Severity::Error,
                                  "SH1117",
                                  "A bound component requires a name.",
                                  {},
                                  node.range});
        if (!node.id.empty() && !symbols.emplace(node.id, &node).second)
          diagnostics_.push_back({path_,
                                  Severity::Error,
                                  "SH1114",
                                  "Duplicate component name " + node.id + ".",
                                  {},
                                  node.range});
        self(self, node.children);
      }
    };
    collect(collect, nodes);
    const auto validate = [&](const auto &self,
                              const std::vector<Node> &values) -> void {
      for (const auto &node : values) {
        const auto label = node.properties.find("label");
        if ((node.name == "Button" || node.name == "TextInput" ||
             node.name == "Number" || node.name == "Range" || node.name == "Checkbox" ||
             node.name == "Select" || node.name == "TextArea") &&
            (label == node.properties.end() || label->second.empty()))
          diagnostics_.push_back({path_,
                                  Severity::Error,
                                  "SH1115",
                                  node.name + " requires an accessible label.",
                                  {},
                                  node.range});
        if (node.name == "Image" && node.properties.count("alt") == 0U)
          diagnostics_.push_back({path_,
                                  Severity::Error,
                                  "SH1115",
                                  "Image requires alt text.",
                                  {},
                                  node.range});
        if (node.name == "Element") {
          static const std::set<std::string> allowed = {
              "article", "aside", "details", "div", "footer", "form", "header",
              "main", "nav", "section", "summary"};
          const auto property = node.properties.find("tag");
          const auto tag = Lower(property == node.properties.end() ? "div" : property->second);
          if (allowed.count(tag) == 0U)
            diagnostics_.push_back({path_,
                                    Severity::Error,
                                    "SH1118",
                                    "Element tag is not a supported semantic container: " +
                                        tag + ".",
                                    "Use a safe container tag such as article, footer, main, nav, or section.",
                                    node.range});
        }
        for (const auto &argument : node.binding.arguments) {
          if (!StartsWith(argument, "$literal$") && !argument.empty() &&
              (std::isalpha(static_cast<unsigned char>(argument.front())) != 0 ||
               argument.front() == '_') &&
              argument != "true" && argument != "false" &&
              symbols.count(argument) == 0U)
            diagnostics_.push_back(
                {path_,
                 Severity::Error,
                 "SH1116",
                 "Binding references unknown component " + argument + ".",
                 {},
                 node.range});
        }
        if (!node.binding.output.empty() && symbols.count(node.binding.output) == 0U)
          diagnostics_.push_back(
              {path_,
               Severity::Error,
               "SH1116",
               "Binding references unknown output " + node.binding.output + ".",
               {},
               node.range});
        self(self, node.children);
      }
    };
    validate(validate, nodes);
  }

  std::vector<Token> tokens_;
  std::size_t position_ = 0;
  std::filesystem::path path_;
  std::vector<Diagnostic> diagnostics_;
};

struct FrontMatterResult {
  Metadata metadata;
  std::string body;
  std::size_t bodyLine = 0;
  std::vector<Diagnostic> diagnostics;
};

auto ParseFrontMatter(const std::string &source, const std::filesystem::path &path,
                      Metadata defaults) -> FrontMatterResult {
  FrontMatterResult result;
  result.metadata = std::move(defaults);
  result.body = source;
  if (!(StartsWith(source, "---\n") || StartsWith(source, "---\r\n")))
    return result;
  const auto lines = SplitLines(source);
  std::size_t close = 1;
  while (close < lines.size() && lines[close].text != "---")
    ++close;
  if (close == lines.size()) {
    result.diagnostics.push_back({path,
                                  Severity::Error,
                                  "SH1021",
                                  "YAML front matter is not closed.",
                                  {},
                                  {{0, 0}, {0, 3}}});
    result.body.clear();
    return result;
  }
  std::ostringstream yamlSource;
  for (std::size_t index = 1; index < close; ++index)
    yamlSource << lines[index].text << '\n';
  try {
    const auto yaml = YAML::Load(yamlSource.str());
    const std::set<std::string> fields = {
        "title",     "description", "keywords", "draft", "date",   "tags",
        "canonical", "permalink",   "noindex",  "toc",   "runtime"};
    if (yaml && !yaml.IsMap())
      throw YAML::ParserException(YAML::Mark{}, "front matter must be a mapping");
    if (yaml) {
      for (const auto &entry : yaml) {
        const auto key = entry.first.as<std::string>();
        if (fields.count(key) == 0U) {
          const auto mark = entry.first.Mark();
          const Position start{static_cast<std::size_t>(std::max(0, mark.line + 1)),
                               static_cast<std::size_t>(std::max(0, mark.column))};
          const Position end{start.line, start.character + key.size()};
          result.diagnostics.push_back({path,
                                        Severity::Error,
                                        "SH1022",
                                        "Unknown front matter field " + key + ".",
                                        {},
                                        {start, end}});
        }
      }
      if (yaml["title"])
        result.metadata.pageTitle = yaml["title"].as<std::string>();
      if (yaml["description"])
        result.metadata.pageDescription = yaml["description"].as<std::string>();
      if (yaml["draft"])
        result.metadata.draft = yaml["draft"].as<bool>();
      if (yaml["date"])
        result.metadata.date = yaml["date"].as<std::string>();
      if (yaml["canonical"])
        result.metadata.canonical = yaml["canonical"].as<std::string>();
      if (yaml["permalink"])
        result.metadata.permalink = yaml["permalink"].as<std::string>();
      if (yaml["noindex"])
        result.metadata.noindex = yaml["noindex"].as<bool>();
      if (yaml["toc"])
        result.metadata.toc = yaml["toc"].as<bool>();
      if (yaml["tags"])
        for (const auto &tag : yaml["tags"])
          result.metadata.tags.push_back(tag.as<std::string>());
      if (yaml["keywords"]) {
        if (yaml["keywords"].IsSequence()) {
          std::ostringstream keywords;
          for (std::size_t index = 0; index < yaml["keywords"].size(); ++index) {
            if (index != 0)
              keywords << ", ";
            keywords << yaml["keywords"][index].as<std::string>();
          }
          result.metadata.keywords = keywords.str();
        } else {
          result.metadata.keywords = yaml["keywords"].as<std::string>();
        }
      }
      if (yaml["runtime"]) {
        if (!yaml["runtime"].IsMap())
          throw YAML::ParserException(yaml["runtime"].Mark(),
                                      "runtime must be a mapping");
        const std::set<std::string> runtimeFields = {"module", "application", "canvas"};
        for (const auto &entry : yaml["runtime"]) {
          const auto key = entry.first.as<std::string>();
          if (runtimeFields.count(key) == 0U) {
            const auto mark = entry.first.Mark();
            const Position start{static_cast<std::size_t>(std::max(0, mark.line + 1)),
                                 static_cast<std::size_t>(std::max(0, mark.column))};
            const Position end{start.line, start.character + key.size()};
            result.diagnostics.push_back({path,
                                          Severity::Error,
                                          "SH1022",
                                          "Unknown runtime field " + key + ".",
                                          {},
                                          {start, end}});
          }
        }
        if (yaml["runtime"]["module"])
          result.metadata.runtime.module = yaml["runtime"]["module"].as<std::string>();
        if (yaml["runtime"]["application"])
          result.metadata.runtime.application =
              yaml["runtime"]["application"].as<std::string>();
        if (yaml["runtime"]["canvas"])
          result.metadata.runtime.canvas = yaml["runtime"]["canvas"].as<std::string>();
      }
    }
  } catch (const YAML::Exception &error) {
    const Position start{static_cast<std::size_t>(std::max(0, error.mark.line + 1)),
                         static_cast<std::size_t>(std::max(0, error.mark.column))};
    const Position end{start.line, start.character + 1};
    result.diagnostics.push_back(
        {path,
         Severity::Error,
         "SH1023",
         "Invalid YAML front matter: " + std::string(error.what()),
         {},
         {start, end}});
  }
  result.bodyLine = close + 1;
  std::ostringstream body;
  for (std::size_t index = result.bodyLine; index < lines.size(); ++index)
    body << lines[index].text << '\n';
  result.body = body.str();
  return result;
}

auto CollectExports(const std::vector<Node> &nodes, std::vector<std::string> &exports)
    -> void {
  for (const auto &node : nodes) {
    if (!node.binding.function.empty() &&
        std::find(exports.begin(), exports.end(), node.binding.function) ==
            exports.end())
      exports.push_back(node.binding.function);
    CollectExports(node.children, exports);
  }
}

auto ContainsComponent(const std::vector<Node> &nodes, const std::string &id,
                       const std::string &type = {}) -> bool {
  for (const auto &node : nodes)
    if ((node.id == id && (type.empty() || node.name == type)) ||
        ContainsComponent(node.children, id, type))
      return true;
  return false;
}

auto AddMarkdown(Document &document, const std::vector<Line> &lines, std::size_t begin,
                 std::size_t end) -> void {
  if (begin >= end)
    return;
  std::ostringstream text;
  for (std::size_t index = begin; index < end; ++index)
    text << lines[index].text << '\n';
  if (Trim(text.str()).empty())
    return;
  Node markdown;
  markdown.kind = NodeKind::Markdown;
  markdown.name = "markdown";
  markdown.text = text.str();
  markdown.range = {{lines[begin].number, 0},
                    {lines[end - 1].number, Utf16Length(lines[end - 1].text)}};
  document.nodes.push_back(std::move(markdown));
}

auto ParseSlamBody(Document &document, const std::string &body, std::size_t firstLine)
    -> void {
  const auto lines = SplitLines(body, firstLine);
  std::size_t markdownStart = 0;
  for (std::size_t index = 0; index < lines.size();) {
    const auto marker = Trim(lines[index].text);
    const bool ui = marker == "```slam-ui";
    const bool cpp = marker == "```cpp wasm";
    const bool unsupported = marker == "```csharp wasm" || marker == "```python wasm";
    if (!ui && !cpp && !unsupported) {
      ++index;
      continue;
    }
    AddMarkdown(document, lines, markdownStart, index);
    const auto open = index++;
    const auto contentStart = index;
    while (index < lines.size() && Trim(lines[index].text) != "```")
      ++index;
    if (index == lines.size()) {
      document.diagnostics.push_back(
          {document.path,
           Severity::Error,
           "SH1100",
           std::string(ui ? "Slam UI" : "Executable C++") + " fence is not closed.",
           {},
           {{lines[open].number, 0},
            {lines[open].number, Utf16Length(lines[open].text)}}});
      return;
    }
    std::ostringstream source;
    for (std::size_t line = contentStart; line < index; ++line)
      source << lines[line].text << '\n';
    if (unsupported) {
      document.diagnostics.push_back(
          {document.path,
           Severity::Error,
           "SH1127",
           "The " + marker.substr(3, marker.size() - 8) +
               " behavior adapter is reserved but not available in " +
               std::string(SLAMHAUS_VERSION) + ".",
           "Use cpp wasm for executable behavior.",
           {{lines[open].number, 0},
            {lines[open].number, Utf16Length(lines[open].text)}}});
    } else if (ui) {
      auto parsed = UiParser(source.str(), document.path,
                             contentStart < lines.size() ? lines[contentStart].number
                                                         : lines[open].number + 1)
                        .Run();
      document.nodes.insert(document.nodes.end(),
                            std::make_move_iterator(parsed.first.begin()),
                            std::make_move_iterator(parsed.first.end()));
      document.diagnostics.insert(document.diagnostics.end(),
                                  std::make_move_iterator(parsed.second.begin()),
                                  std::make_move_iterator(parsed.second.end()));
    } else if (!document.behavior.source.empty()) {
      document.diagnostics.push_back(
          {document.path,
           Severity::Error,
           "SH1120",
           "A .slam page may contain only one executable fence.",
           "Move shared code into code/.",
           {{lines[open].number, 0},
            {lines[open].number, Utf16Length(lines[open].text)}}});
    } else {
      document.behavior.language = "cpp";
      document.behavior.source = source.str();
      document.behavior.range = {{lines[open].number, 0}, {lines[index].number, 3}};
    }
    ++index;
    markdownStart = index;
  }
  AddMarkdown(document, lines, markdownStart, lines.size());
  CollectExports(document.nodes, document.behavior.exports);
  if (!document.behavior.exports.empty() && document.behavior.source.empty() &&
      document.metadata.runtime.module.empty())
    document.diagnostics.push_back(
        {document.path,
         Severity::Error,
         "SH1121",
         "Interactive UI requires embedded C++ or runtime.module.",
         {},
         document.nodes.empty() ? Range{} : document.nodes.front().range});
  if (!document.behavior.source.empty() && !document.metadata.runtime.module.empty())
    document.diagnostics.push_back(
        {document.path,
         Severity::Error,
         "SH1122",
         "Embedded C++ and runtime.module cannot be combined.",
         {},
         document.behavior.range});
  if (!document.metadata.runtime.application.empty() &&
      document.metadata.runtime.canvas.empty())
    document.diagnostics.push_back({document.path,
                                    Severity::Error,
                                    "SH1123",
                                    "runtime.application requires runtime.canvas.",
                                    {},
                                    document.behavior.range});
  if (!document.metadata.runtime.canvas.empty() &&
      !ContainsComponent(document.nodes, document.metadata.runtime.canvas, "Canvas"))
    document.diagnostics.push_back({document.path,
                                    Severity::Error,
                                    "SH1128",
                                    "runtime.canvas must name a Canvas component.",
                                    {},
                                    document.behavior.range});
  if (!document.metadata.runtime.application.empty() &&
      document.behavior.source.empty() && document.metadata.runtime.module.empty())
    document.diagnostics.push_back(
        {document.path,
         Severity::Error,
         "SH1121",
         "An application requires embedded C++ or runtime.module.",
         {},
         document.behavior.range});
}

auto MarkdownText(cmark_node *node) -> std::string {
  std::string text;
  if (const auto *literal = cmark_node_get_literal(node); literal != nullptr)
    text += literal;
  for (auto *child = cmark_node_first_child(node); child != nullptr;
       child = cmark_node_next(child))
    text += MarkdownText(child);
  return text;
}

auto HeadingSlug(const std::string &text) -> std::string {
  std::string slug;
  bool separator = false;
  for (char character : text) {
    if (std::isalnum(static_cast<unsigned char>(character)) != 0) {
      if (separator && !slug.empty())
        slug.push_back('-');
      slug.push_back(
          static_cast<char>(std::tolower(static_cast<unsigned char>(character))));
      separator = false;
    } else {
      separator = true;
    }
  }
  return slug.empty() ? "section" : slug;
}

auto HighlightCode(const std::string &code, const std::string &language)
    -> std::string {
  std::ostringstream html;
  html << "<pre><code class=\"language-" << EscapeHtml(language) << "\">";
  std::size_t cursor = 0;
  for (const auto &token : Syntax::Tokens(code, Lower(language))) {
    if (token.begin < cursor || token.end > code.size())
      continue;
    html << EscapeHtml(code.substr(cursor, token.begin - cursor));
    html << "<span class=\"tok-" << EscapeHtml(token.type) << "\">"
         << EscapeHtml(code.substr(token.begin, token.end - token.begin)) << "</span>";
    cursor = token.end;
  }
  html << EscapeHtml(code.substr(cursor));
  html << "</code></pre>\n";
  return html.str();
}

struct MarkdownResult {
  std::string html;
  std::vector<Diagnostic> diagnostics;
  struct Heading {
    int level = 0;
    std::string id;
    std::string text;
  };
  std::vector<Heading> headings;
};

auto RenderMarkdown(const Node &node, const std::filesystem::path &path,
                    const RenderOptions &options,
                    std::map<std::string, std::size_t> &slugCounts) -> MarkdownResult {
  cmark_gfm_core_extensions_ensure_registered();
  const int flags = options.unsafeHtml ? CMARK_OPT_UNSAFE : CMARK_OPT_DEFAULT;
  cmark_parser *parser = cmark_parser_new(flags);
  for (const auto *name : {"table", "strikethrough", "autolink", "tasklist"})
    if (auto *extension = cmark_find_syntax_extension(name))
      cmark_parser_attach_syntax_extension(parser, extension);
  cmark_parser_feed(parser, node.text.data(), node.text.size());
  cmark_node *root = cmark_parser_finish(parser);
  std::vector<Diagnostic> diagnostics;
  std::vector<cmark_node *> headingNodes;
  std::vector<cmark_node *> codeNodes;
  std::vector<MarkdownResult::Heading> headings;
  cmark_iter *iterator = cmark_iter_new(root);
  while (cmark_iter_next(iterator) != CMARK_EVENT_DONE) {
    if (cmark_iter_get_event_type(iterator) != CMARK_EVENT_ENTER)
      continue;
    cmark_node *current = cmark_iter_get_node(iterator);
    const auto type = cmark_node_get_type(current);
    const Range range{
        {node.range.start.line +
             static_cast<std::size_t>(std::max(1, cmark_node_get_start_line(current)) -
                                      1),
         static_cast<std::size_t>(std::max(1, cmark_node_get_start_column(current)) -
                                  1)},
        {node.range.start.line + static_cast<std::size_t>(
                                     std::max(1, cmark_node_get_end_line(current)) - 1),
         static_cast<std::size_t>(std::max(1, cmark_node_get_end_column(current)))}};
    if (!options.unsafeHtml &&
        (type == CMARK_NODE_HTML_BLOCK || type == CMARK_NODE_HTML_INLINE))
      diagnostics.push_back({path, Severity::Error, "SH1020",
                             "Raw HTML is disabled for this project.",
                             "Set build.unsafe_html only for trusted content.", range});
    if (type == CMARK_NODE_LINK || type == CMARK_NODE_IMAGE) {
      const char *url = cmark_node_get_url(current);
      if (url != nullptr) {
        const auto resolved =
            options.resolveUrl ? options.resolveUrl(url) : AdjustLink(url);
        cmark_node_set_url(current, resolved.c_str());
      }
    }
    if (type == CMARK_NODE_HEADING)
      headingNodes.push_back(current);
    if (type == CMARK_NODE_CODE_BLOCK &&
        cmark_node_get_fence_info(current) != nullptr &&
        *cmark_node_get_fence_info(current) != '\0')
      codeNodes.push_back(current);
  }
  cmark_iter_free(iterator);
  for (auto *heading : headingNodes) {
    const auto text = MarkdownText(heading);
    const auto base = HeadingSlug(text);
    const auto count = slugCounts[base]++;
    const auto id = count == 0 ? base : base + '-' + std::to_string(count + 1);
    const auto level = cmark_node_get_heading_level(heading);
    auto *custom = cmark_node_new(CMARK_NODE_CUSTOM_BLOCK);
    const auto open = "<h" + std::to_string(level) + " id=\"" + EscapeHtml(id) + "\">";
    const auto close = "</h" + std::to_string(level) + ">\n";
    cmark_node_set_on_enter(custom, open.c_str());
    cmark_node_set_on_exit(custom, close.c_str());
    cmark_node_insert_before(heading, custom);
    while (auto *child = cmark_node_first_child(heading)) {
      cmark_node_unlink(child);
      cmark_node_append_child(custom, child);
    }
    cmark_node_free(heading);
    headings.push_back({level, id, text});
  }
  for (auto *code : codeNodes) {
    const auto *literal = cmark_node_get_literal(code);
    const auto *info = cmark_node_get_fence_info(code);
    auto *custom = cmark_node_new(CMARK_NODE_CUSTOM_BLOCK);
    const auto highlighted = HighlightCode(literal == nullptr ? "" : literal,
                                           info == nullptr ? "text" : Trim(info));
    cmark_node_set_on_enter(custom, highlighted.c_str());
    cmark_node_set_on_exit(custom, "");
    cmark_node_insert_before(code, custom);
    cmark_node_free(code);
  }
  char *rendered =
      cmark_render_html(root, flags, cmark_parser_get_syntax_extensions(parser));
  std::string html = rendered == nullptr ? "" : rendered;
  std::free(rendered);
  cmark_node_free(root);
  cmark_parser_free(parser);
  return {std::move(html), std::move(diagnostics), std::move(headings)};
}

auto Property(const Node &node, const std::string &name,
              const std::string &fallback = {}) -> std::string {
  const auto found = node.properties.find(name);
  return found == node.properties.end() ? fallback : found->second;
}

auto IsTrue(const Node &node, const std::string &name) -> bool {
  return Property(node, name) == "true";
}

auto EventName(const std::string &event) -> std::string {
  const auto lowered = Lower(event);
  return lowered == "submit" ? "submit" : lowered;
}

struct Renderer {
  explicit Renderer(const Document &source, RenderOptions renderOptions)
      : document(source), options(std::move(renderOptions)) {}

  const Document &document;
  RenderOptions options;
  std::ostringstream html;
  std::vector<Diagnostic> diagnostics;
  std::map<std::string, std::size_t> slugCounts;
  std::vector<MarkdownResult::Heading> headings;
  nlohmann::json bindings = nlohmann::json::array();

  auto Resolve(const std::string &value) const -> std::string {
    return options.resolveUrl ? options.resolveUrl(value) : AdjustLink(value);
  }

  auto RenderNodes(const std::vector<Node> &nodes) -> void {
    for (const auto &node : nodes) {
      if (node.kind == NodeKind::Markdown) {
        auto rendered = RenderMarkdown(node, document.path, options, slugCounts);
        html << rendered.html;
        diagnostics.insert(diagnostics.end(), rendered.diagnostics.begin(),
                           rendered.diagnostics.end());
        headings.insert(headings.end(), rendered.headings.begin(),
                        rendered.headings.end());
      } else {
        RenderComponent(node);
      }
    }
  }

  auto EmitCommon(const Node &node) -> void {
    if (!node.id.empty())
      html << " id=\"" << SafeId(node.id) << "\"";
    if (!Property(node, "class").empty())
      html << " class=\"" << EscapeHtml(Property(node, "class")) << "\"";
  }

  auto EmitBinding(const Node &node) -> void {
    if (node.binding.function.empty())
      return;
    nlohmann::json binding = {{"event", EventName(node.binding.event)},
                              {"function", node.binding.function},
                              {"target", "#" + SafeId(node.id)},
                              {"args", nlohmann::json::array()},
                              {"output", node.binding.output.empty()
                                             ? ""
                                             : "#" + SafeId(node.binding.output)}};
    for (const auto &argument : node.binding.arguments) {
      if (!StartsWith(argument, "$literal$") && !argument.empty() &&
          (std::isalpha(static_cast<unsigned char>(argument.front())) != 0 ||
           argument.front() == '_') &&
          argument != "true" && argument != "false")
        binding["args"].push_back("#" + SafeId(argument));
      else
        binding["args"].push_back(
            {{"literal",
              StartsWith(argument, "$literal$") ? argument.substr(9) : argument}});
    }
    bindings.push_back(std::move(binding));
  }

  auto RenderComponent(const Node &node) -> void {
    const auto *spec = FindSpec(node.name);
    if (spec == nullptr)
      return;
    if (node.name == "Include") {
      html << "<!-- slamhaus include:" << EscapeHtml(Property(node, "name"))
           << " -->\n";
      return;
    }
    if (node.name == "Text") {
      html << "<span";
      EmitCommon(node);
      html << '>' << EscapeHtml(Property(node, "text")) << "</span>\n";
      return;
    }
    if (node.name == "Output") {
      html << "<output";
      EmitCommon(node);
      html << " aria-live=\"" << EscapeHtml(Property(node, "live", "polite")) << "\">"
           << EscapeHtml(Property(node, "text")) << "</output>\n";
      return;
    }
    if (node.name == "Image") {
      html << "<img";
      EmitCommon(node);
      html << " src=\"" << EscapeHtml(Resolve(Property(node, "src"))) << "\" alt=\""
           << EscapeHtml(Property(node, "alt")) << "\"";
      for (const auto *property : {"width", "height"})
        if (!Property(node, property).empty())
          html << ' ' << property << "=\"" << EscapeHtml(Property(node, property))
               << "\"";
      html << ">\n";
      return;
    }
    if (node.name == "Audio" || node.name == "Video") {
      const auto tag = Lower(node.name);
      html << '<' << tag;
      EmitCommon(node);
      html << " src=\"" << EscapeHtml(Resolve(Property(node, "src"))) << "\"";
      for (const auto *property :
           {"controls", "loop", "muted", "autoplay", "playsinline"})
        if (IsTrue(node, property))
          html << ' ' << property;
      html << "></" << tag << ">\n";
      return;
    }
    if (node.name == "Canvas") {
      html << "<canvas";
      EmitCommon(node);
      for (const auto *property : {"width", "height"})
        if (!Property(node, property).empty())
          html << ' ' << property << "=\"" << EscapeHtml(Property(node, property))
               << "\"";
      if (Property(node, "responsive", "true") == "true")
        html << " data-slam-responsive";
      html << "></canvas>\n";
      return;
    }
    if (node.name == "Button") {
      html << "<button type=\"" << EscapeHtml(Property(node, "type", "button")) << "\"";
      EmitCommon(node);
      html << '>' << EscapeHtml(Property(node, "label")) << "</button>\n";
      EmitBinding(node);
      return;
    }
    if (node.name == "TextInput" || node.name == "Number" || node.name == "Range" ||
        node.name == "Checkbox") {
      const auto type = node.name == "TextInput" ? "text" : Lower(node.name);
      html << "<label>" << EscapeHtml(Property(node, "label")) << "<input type=\""
           << type << "\"";
      EmitCommon(node);
      for (const auto *property :
           {"name", "value", "placeholder", "min", "max", "step"})
        if (!Property(node, property).empty())
          html << ' ' << property << "=\"" << EscapeHtml(Property(node, property))
               << "\"";
      for (const auto *property : {"required", "disabled", "checked"})
        if (IsTrue(node, property))
          html << ' ' << property;
      html << "></label>\n";
      EmitBinding(node);
      return;
    }
    if (node.name == "TextArea") {
      html << "<label>" << EscapeHtml(Property(node, "label")) << "<textarea";
      EmitCommon(node);
      for (const auto *property : {"name", "placeholder", "rows"})
        if (!Property(node, property).empty())
          html << ' ' << property << "=\"" << EscapeHtml(Property(node, property))
               << "\"";
      html << '>' << EscapeHtml(Property(node, "value")) << "</textarea></label>\n";
      EmitBinding(node);
      return;
    }
    if (node.name == "Select") {
      html << "<label>" << EscapeHtml(Property(node, "label")) << "<select";
      EmitCommon(node);
      html << ">";
      std::istringstream values(Property(node, "options"));
      std::string value;
      while (std::getline(values, value))
        html << "<option" << (value == Property(node, "value") ? " selected" : "")
             << '>' << EscapeHtml(value) << "</option>";
      html << "</select></label>\n";
      EmitBinding(node);
      return;
    }
    std::string tag = spec->tag;
    if (node.name == "Element") {
      static const std::set<std::string> allowed = {
          "article", "aside", "details", "div", "footer", "form", "header", "main",
          "nav", "section", "summary"};
      tag = Lower(Property(node, "tag", "div"));
      if (allowed.count(tag) == 0U)
        tag = "div";
    }
    html << '<' << tag;
    EmitCommon(node);
    if (node.name == "Row" || node.name == "Column")
      html << " data-slam-layout=\"" << Lower(node.name) << "\"";
    if (node.name == "Panel" && !Property(node, "label").empty())
      html << " aria-label=\"" << EscapeHtml(Property(node, "label")) << "\"";
    if (node.name == "Form") {
      html << " method=\"" << EscapeHtml(Property(node, "method", "post")) << "\"";
      if (!Property(node, "action").empty())
        html << " action=\"" << EscapeHtml(Resolve(Property(node, "action"))) << "\"";
    }
    html << ">\n";
    RenderNodes(node.children);
    html << "</" << tag << ">\n";
    EmitBinding(node);
  }

  auto Finish() -> RenderResult {
    RenderNodes(document.nodes);
    if (!document.metadata.toc) {
      // No table of contents requested.
    } else if (!headings.empty()) {
      std::ostringstream toc;
      toc << "<nav class=\"table-of-contents\" aria-label=\"Table of contents\"><ol>";
      for (const auto &heading : headings)
        toc << "<li class=\"toc-level-" << heading.level << "\"><a href=\"#"
            << EscapeHtml(heading.id) << "\">" << EscapeHtml(heading.text)
            << "</a></li>";
      toc << "</ol></nav>\n";
      const auto value = toc.str() + html.str();
      html.str({});
      html.clear();
      html << value;
    }
    const auto module = !options.moduleUrl.empty() ? options.moduleUrl
                                                   : document.metadata.runtime.module;
    if (!document.metadata.runtime.application.empty())
      bindings.push_back({{"application", true},
                          {"canvas", "#" + SafeId(document.metadata.runtime.canvas)}});
    if (!bindings.empty()) {
      for (auto &binding : bindings) {
        binding["module"] = module;
        if (!binding.contains("application"))
          binding["application"] = false;
      }
      auto json = bindings.dump();
      for (std::size_t position = 0;
           (position = json.find('<', position)) != std::string::npos; position += 6)
        json.replace(position, 1, "\\u003c");
      html << "<script type=\"application/json\" data-slamhaus-bindings>" << json
           << "</script>\n<script type=\"module\" src=\""
           << EscapeHtml(options.runtimeUrl) << "\"></script>\n";
    }
    return {html.str(), std::move(diagnostics)};
  }
};

} // namespace

auto Parse(const std::string &source, const std::filesystem::path &path,
           Metadata defaults) -> Document {
  auto frontMatter = ParseFrontMatter(source, path, std::move(defaults));
  Document document;
  document.path = path;
  document.metadata = std::move(frontMatter.metadata);
  document.diagnostics = std::move(frontMatter.diagnostics);
  if (path.extension() == ".slam") {
    ParseSlamBody(document, frontMatter.body, frontMatter.bodyLine);
  } else {
    Node markdown;
    markdown.kind = NodeKind::Markdown;
    markdown.name = "markdown";
    markdown.text = frontMatter.body;
    markdown.range = {{frontMatter.bodyLine, 0},
                      {frontMatter.bodyLine + SplitLines(frontMatter.body).size(), 0}};
    document.nodes.push_back(std::move(markdown));
  }
  if (document.metadata.pageTitle.empty()) {
    for (const auto &line : SplitLines(frontMatter.body)) {
      const auto text = Trim(line.text);
      if (StartsWith(text, "# ")) {
        document.metadata.pageTitle = text.substr(2);
        break;
      }
    }
  }
  return document;
}

auto Render(const Document &document, const RenderOptions &options) -> RenderResult {
  auto result = Renderer(document, options).Finish();
  result.diagnostics.insert(result.diagnostics.begin(), document.diagnostics.begin(),
                            document.diagnostics.end());
  return result;
}

auto Format(const std::string &source) -> std::string {
  // Markdown and C++ remain author-owned. Slam UI is already deliberately strict,
  // so formatting is idempotent until the structured formatter lands.
  return source;
}

auto SemanticTokens(const std::string &source, const std::filesystem::path &path)
    -> std::vector<SemanticToken> {
  std::vector<SemanticToken> result;
  const auto lines = SplitLines(source);
  std::string fence;
  for (const auto &line : lines) {
    const auto trimmed = Trim(line.text);
    if (StartsWith(trimmed, "```")) {
      if (fence.empty())
        fence = Trim(trimmed.substr(3));
      else
        fence.clear();
      result.push_back(
          {{{line.number, 0}, {line.number, Utf16Length(line.text)}}, "operator"});
      continue;
    }
    if (line.number == 0 && trimmed == "---") {
      result.push_back({{{line.number, 0}, {line.number, 3}}, "operator"});
      continue;
    }
    if (fence == "slam-ui") {
      for (std::size_t index = 0; index < line.text.size();) {
        if (std::isalpha(static_cast<unsigned char>(line.text[index])) == 0 &&
            line.text[index] != '_') {
          ++index;
          continue;
        }
        const auto begin = index++;
        while (index < line.text.size() &&
               (std::isalnum(static_cast<unsigned char>(line.text[index])) != 0 ||
                line.text[index] == '_'))
          ++index;
        const auto word = line.text.substr(begin, index - begin);
        const auto *spec = FindSpec(word);
        const auto type = spec != nullptr ? "type"
                          : word == "on"  ? "keyword"
                                          : "variable";
        result.push_back({{{line.number, begin}, {line.number, index}}, type});
      }
    } else if (fence == "cpp wasm" || fence == "cpp") {
      static const std::set<std::string> cppKeywords = {
          "auto",   "bool",   "class",     "const",    "final",
          "float",  "int",    "namespace", "override", "private",
          "public", "return", "struct",    "using",    "void"};
      for (std::size_t index = 0; index < line.text.size();) {
        if (std::isalpha(static_cast<unsigned char>(line.text[index])) == 0 &&
            line.text[index] != '_') {
          ++index;
          continue;
        }
        const auto begin = index++;
        while (index < line.text.size() &&
               (std::isalnum(static_cast<unsigned char>(line.text[index])) != 0 ||
                line.text[index] == '_'))
          ++index;
        const auto word = line.text.substr(begin, index - begin);
        if (cppKeywords.count(word) != 0U)
          result.push_back({{{line.number, begin}, {line.number, index}}, "keyword"});
      }
    } else if (fence.empty() && StartsWith(trimmed, "#")) {
      result.push_back(
          {{{line.number, 0}, {line.number, Utf16Length(line.text)}}, "namespace"});
    }
  }
  (void)path;
  return result;
}

auto HasErrors(const std::vector<Diagnostic> &diagnostics) -> bool {
  return std::any_of(
      diagnostics.begin(), diagnostics.end(),
      [](const auto &diagnostic) { return diagnostic.severity == Severity::Error; });
}

auto DiagnosticsJson(const std::vector<Diagnostic> &diagnostics) -> nlohmann::json {
  nlohmann::json values = nlohmann::json::array();
  for (const auto &diagnostic : diagnostics)
    values.push_back({{"path", diagnostic.path.generic_string()},
                      {"severity", SeverityName(diagnostic.severity)},
                      {"code", diagnostic.code},
                      {"message", diagnostic.message},
                      {"help", diagnostic.help},
                      {"range",
                       {{"start",
                         {{"line", diagnostic.range.start.line},
                          {"character", diagnostic.range.start.character}}},
                        {"end",
                         {{"line", diagnostic.range.end.line},
                          {"character", diagnostic.range.end.character}}}}}});
  return {{"schema", 1}, {"diagnostics", std::move(values)}};
}

auto SchemaJson() -> nlohmann::json {
  nlohmann::json components = nlohmann::json::array();
  for (const auto &spec : ComponentSpecs())
    components.push_back(
        {{"name", spec.name},
         {"container", spec.container},
         {"properties", spec.properties},
         {"defaultEvent", spec.defaultEvent == nullptr ? "" : spec.defaultEvent}});
  return {{"schema", 4},
          {"extensions", {{"pages", {".slam", ".md"}}}},
          {"markdown", {{"dialect", "gfm"}, {"enhanced", ".slam"}}},
          {"frontMatter",
           {{"format", "yaml"},
            {"fields",
             {"title", "description", "keywords", "draft", "date", "tags", "canonical",
              "permalink", "noindex", "toc", "runtime"}}}},
          {"fences", {{"ui", "slam-ui"}, {"behavior", "cpp wasm"}}},
          {"syntax", {{"style", "cpp-blocks"},
                      {"properties", "assignment"},
                      {"events", "on Event => Function(args) -> Output"}}},
          {"components", std::move(components)}};
}

auto EscapeHtml(const std::string &value) -> std::string {
  std::string result;
  result.reserve(value.size());
  for (char character : value) {
    switch (character) {
    case '&':
      result += "&amp;";
      break;
    case '<':
      result += "&lt;";
      break;
    case '>':
      result += "&gt;";
      break;
    case '"':
      result += "&quot;";
      break;
    case '\'':
      result += "&#39;";
      break;
    default:
      result.push_back(character);
    }
  }
  return result;
}

auto AdjustLink(const std::string &value) -> std::string {
  const auto suffix = value.find_first_of("?#");
  std::string base = suffix == std::string::npos ? value : value.substr(0, suffix);
  const auto tail = suffix == std::string::npos ? "" : value.substr(suffix);
  const auto extension = std::filesystem::path(base).extension().string();
  if (extension == ".slam" || extension == ".md")
    base = std::filesystem::path(base).replace_extension(".html").generic_string();
  return base + tail;
}

} // namespace Language
