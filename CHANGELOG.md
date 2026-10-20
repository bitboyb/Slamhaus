# Changelog

## 0.1.0

- Redesign Slamhaus as portable GFM plus an explicit C-like `slam-ui` language.
- Add page-local `cpp wasm` fences with injected SDK setup, inferred modules,
  automatic typed exports, application registration, and source-line mapping.
- Replace selector and module boilerplate with named UI symbols, typed properties,
  default events, accessible HTML, validated output targets, and safe components.
- Replace `content/include/config.ini` with validated root `slamhaus.yaml` and the
  `pages/`, optional `partials/`, `assets/`, `code/`, and generated `dist/` layout.
- Make `init` create a genuinely blank universal project and add baked
  `examples --profile minimal|complete` projects for static and WASM users.
- Add pretty logical routes, base-path-aware site-root links, traversal and route
  validation, atomic output replacement, PWA files, sitemap, and optional feeds.
- Add build-time Tree-sitter highlighting for C++, C#, and Python fences without
  client-side JavaScript.
- Expand the editor-neutral LSP with Slam UI and metadata completion, semantic
  tokens across Markdown/UI/C++, formatting, hover, symbols, and diagnostics.
- Add a filesystem-free workspace compiler and language-neutral module manifests
  as the boundary for a future local-file browser editor and C#/Python adapters.
- Retain the Unreal-familiar, renderer-neutral C++ SDK for input, touch, gamepads,
  display, DOM, audio, assets, storage, navigation, accessibility, sensors, PWA,
  and mobile application lifecycle.
- Expand parser, renderer, configuration, routing, partial, CLI, LSP, SDK,
  rollback, deterministic-build, sanitizer, and real Emscripten coverage.
- Keep one command-only CLI repository with binary GitHub releases and no
  editor-specific extension or handwritten JavaScript requirement.

This 0.1.0 release intentionally removes the intermediate directive syntax,
`.smd`, `config.ini`, `@wasm`, the original numbered menu, and compatibility
aliases. Existing experimental projects require manual conversion.
