#include "syntax/syntax.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <set>
#include <string_view>

#include <tree_sitter/api.h>

extern "C" {
const TSLanguage *tree_sitter_cpp();
const TSLanguage *tree_sitter_python();
const TSLanguage *tree_sitter_c_sharp();
}

namespace Syntax {
namespace {

auto Language(const std::string &name) -> const TSLanguage * {
  if (name == "cpp" || name == "c++" || name == "cc" || name == "cxx")
    return tree_sitter_cpp();
  if (name == "python" || name == "py")
    return tree_sitter_python();
  if (name == "csharp" || name == "cs" || name == "c#")
    return tree_sitter_c_sharp();
  return nullptr;
}

auto TokenType(TSNode node) -> std::string {
  const std::string_view type = ts_node_type(node);
  if (type.find("comment") != std::string_view::npos)
    return "comment";
  if (type.find("string") != std::string_view::npos || type == "character_literal")
    return "string";
  if (type.find("number") != std::string_view::npos || type == "integer" ||
      type == "float")
    return "number";
  if (type == "preproc_include" || type.find("preproc") != std::string_view::npos ||
      type == "decorator")
    return "meta";
  if (type == "primitive_type" || type == "type_identifier")
    return "type";
  static const std::set<std::string_view> keywords = {
      "auto",  "async",    "await",   "break",     "case",    "catch",     "class",
      "const", "continue", "def",     "default",   "do",      "elif",      "else",
      "enum",  "except",   "false",   "finally",   "for",     "foreach",   "from",
      "if",    "import",   "in",      "interface", "is",      "lambda",    "namespace",
      "new",   "None",     "nullptr", "override",  "private", "protected", "public",
      "raise", "return",   "static",  "struct",    "switch",  "template",  "this",
      "throw", "true",     "try",     "using",     "virtual", "while",     "with",
      "yield"};
  if (keywords.count(type) != 0U)
    return "keyword";
  return {};
}

auto Collect(TSNode node, std::vector<Token> &tokens) -> void {
  const auto type = TokenType(node);
  if (!type.empty()) {
    tokens.push_back({ts_node_start_byte(node), ts_node_end_byte(node), type});
    return;
  }
  const auto count = ts_node_child_count(node);
  for (std::uint32_t index = 0; index < count; ++index)
    Collect(ts_node_child(node, index), tokens);
}

} // namespace

auto Tokens(const std::string &source, const std::string &language)
    -> std::vector<Token> {
  const auto *grammar = Language(language);
  if (grammar == nullptr)
    return {};
  TSParser *parser = ts_parser_new();
  if (!ts_parser_set_language(parser, grammar)) {
    ts_parser_delete(parser);
    return {};
  }
  TSTree *tree = ts_parser_parse_string(parser, nullptr, source.data(),
                                        static_cast<std::uint32_t>(source.size()));
  std::vector<Token> tokens;
  if (tree != nullptr) {
    Collect(ts_tree_root_node(tree), tokens);
    ts_tree_delete(tree);
  }
  ts_parser_delete(parser);
  std::sort(tokens.begin(), tokens.end(), [](const auto &left, const auto &right) {
    return left.begin < right.begin ||
           (left.begin == right.begin && left.end < right.end);
  });
  tokens.erase(std::unique(tokens.begin(), tokens.end(),
                           [](const auto &left, const auto &right) {
                             return left.begin == right.begin && left.end == right.end;
                           }),
               tokens.end());
  return tokens;
}

} // namespace Syntax
