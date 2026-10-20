#include "language/language.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t *data, std::size_t size) {
  const auto document = Language::Parse(
      std::string(reinterpret_cast<const char *>(data), size), "fuzz.slam");
  (void)Language::Render(document);
  (void)Language::DiagnosticsJson(document.diagnostics);
  return 0;
}
