# AGENTS.md

## Purpose

Slamhaus is a functional C++17 static-site compiler with a typed C++ to
WebAssembly SDK. Keep parsing and rendering deterministic; isolate filesystem,
process, network, and terminal effects in the project, toolchain, and CLI layers.

## Repository map

- `src/language`: source values, parsing, validation, formatting, and HTML.
- `src/core`: project configuration, routing, assets, and atomic builds.
- `src/wasm`: Emscripten discovery, compilation, the baked SDK, and web runtime.
- `src/toolchain`: managed, pinned external toolchains.
- `src/lsp`: editor-neutral language server.
- `src/syntax`: pinned Tree-sitter tokenization shared by HTML and the LSP.
- `sdk/slam`: public C++ API. This header is embedded and installed.
- `tests`: unit and integration behavior; test public contracts, not internals.

## Rules

- Prefer pure transformations, values, standard algorithms, and explicit results.
- Keep the filesystem-free `Workspace` compiler usable by a future browser host.
- Keep `.md` portable GFM. Enhanced behavior belongs only in `.slam` fences.
- Drive Slam UI parsing, rendering, schema, completion, and tests from shared
  component specifications instead of parallel lists.
- Do not add compatibility aliases during `0.0.x`; removed syntax stays rejected.
- Do not add user-authored JavaScript to examples. Extend the SDK or generated
  runtime when a browser capability is generally useful.
- Keep the runtime renderer-agnostic. Do not add actors, reflection, scenes, or a
  proprietary renderer.
- Add or update a stable `SHxxxx` diagnostic for every user-correctable failure.
- Preserve atomic output replacement and project-root path containment.
- Keep generated HTML semantic and accessible; controls need names, images need
  alternative text, and keyboard/mobile interaction cannot be an afterthought.
- Update `VERSION` only once; CMake, packages, and the CLI derive from it.

## Verification

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/slamhaus check . --strict
./build/slamhaus fmt . --check
```

Run clang-tidy and sanitizer builds for changes to parsing, paths, process
execution, the SDK bridge, or shared runtime state. Build a real sample with
Emscripten after changing `sdk/slam/slam.hpp`, `src/wasm`, or compiler flags.
