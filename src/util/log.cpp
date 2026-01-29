#include "log.hpp"
#include <iostream>

namespace Log {
auto Info(const std::string &message) -> void { std::cout << message << "\n"; }

auto Error(const std::string &message) -> void { std::cerr << message << "\n"; }
} // namespace Log
