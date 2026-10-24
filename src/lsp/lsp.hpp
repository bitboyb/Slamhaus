#pragma once

#include <istream>
#include <ostream>

namespace Lsp {

/** Run the editor-neutral JSON-RPC language server until input closes. */
auto Run(std::istream &input, std::ostream &output) -> int;

} // namespace Lsp
