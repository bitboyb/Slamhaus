#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace Syntax {

/** A byte range and token category returned by the Tree-sitter tokenizer. */
struct Token {
  std::size_t begin = 0;
  std::size_t end = 0;
  std::string type;
};

/** Tokenize source using the pinned grammar for the requested language. */
auto Tokens(const std::string &source, const std::string &language)
    -> std::vector<Token>;

} // namespace Syntax
