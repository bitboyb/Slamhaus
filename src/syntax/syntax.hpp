#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace Syntax {

struct Token {
  std::size_t begin = 0;
  std::size_t end = 0;
  std::string type;
};

auto Tokens(const std::string &source, const std::string &language)
    -> std::vector<Token>;

} // namespace Syntax
