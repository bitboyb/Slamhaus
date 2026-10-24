# Slamhaus language reference

## Source formats

`.md` is portable GitHub Flavored Markdown. `.slam` uses the same Markdown and
adds executable Slamhaus fences. Both accept optional YAML front matter:

```yaml
---
title: Release notes
description: Changes in Slamhaus
keywords: [slamhaus, wasm]
date: 2026-08-16
tags: [release, cpp]
draft: false
noindex: false
canonical: https://example.com/releases/current
permalink: /releases/current/
toc: false
runtime:
  module: shared
  application: FGame
  canvas: Game
---
```

Unknown fields are errors. `runtime.module` selects code from `code/` and
cannot be combined with page-local executable C++. `application` requires a
named `Canvas` component.

## Slam UI grammar

Slam UI is declarative and C++-like:

```text
Type [Name](positional) {
  property = value;
  on Event => Function(args) -> Output;
}
```

Values are strings, numbers, booleans, identifiers, or lists. Commas are
required between arguments. `//` and `/* */` comments are supported.

```slam-ui
Column Settings() {
  TextInput Player("Player") { placeholder = "Ada"; }
  Number Lives("Lives") { value = 3; min = 1; }
  Range Volume("Volume") { value = 75; min = 0; max = 100; }
  Checkbox Sound("Sound") { checked = true; }
  Select Difficulty("Difficulty") { options = ["Easy", "Hard"]; }
  Button Save("Save") {
    on Click => Store(Player, Lives, Volume, Sound, Difficulty) -> Status;
  }
  Output Status("Ready");
};
```

Names form a page-local symbol table. Slamhaus generates private DOM IDs;
bindings refer to symbols, never CSS selectors.

### Components

- Layout: `Row`, `Column`, `Panel`, `Form`.
- Controls: `Button`, `TextInput`, `Number`, `Range`, `Checkbox`, `Select`,
  `TextArea`.
- Content: `Text`, `Output`, `Image`, `Audio`, `Video`, `Canvas`.
- Composition: `Include("name")` loads `.md` or `.slam` from `partials/`.
- Escape hatch: `Element("details")` accepts only safe semantic container tags,
  including `article`, `aside`, `details`, `div`, `footer`, `form`, `header`,
  `main`, `nav`, `section`, and `summary`.

Controls require accessible labels and images require `alt`. `Output` is an
ARIA live region. Inline event handlers, unsafe URLs, and `srcdoc` are not part
of the language.

Use configured layout partials for site-wide regions:

```yaml
layout:
  footer: footer
```

Use `Include("name");` inside a `slam-ui` fence when composition belongs inside
the page body. Layout partials are rendered inside semantic `header`, `nav`, or
`footer` elements; inline includes are expanded where they appear.

Default events are `Click` for buttons, `Submit` for forms, `Input` for ranges,
and `Change` for other controls. Use `on Event` to override the default.

## Embedded behavior

`Canvas` provides a semantic, responsive canvas element and the SDK exposes
application lifecycle and input state. Slamhaus 0.1.0 does not yet provide a
portable C++ drawing API; use DOM components for visible examples until a
documented 2D rendering surface is added.

A `.slam` page may contain one executable fence:

```cpp wasm
auto Add(int32 A, int32 B) -> int32 {
  return A + B;
}
```

Slamhaus injects the SDK include and namespace and automatically exports
page-local namespace-scope free functions referenced by Slam UI. Exported
functions cannot be overloaded. Compiler diagnostics map back to the `.slam`
source through `#line` directives.

Ordinary `cpp`, `csharp`, and `python` fences are displayed and highlighted.
`csharp wasm` and `python wasm` are reserved and currently produce a clear
unsupported-adapter diagnostic.

Larger modules live in `code/name.cpp` or `code/name/*.cpp`. Shared code is
normal C++ and uses explicit SDK registration.

## Routing

Pretty routes are enabled by default:

```text
pages/index.slam       -> /
pages/about.md         -> /about/
pages/guide/start.md   -> /guide/start/
```

Markdown links point at source pages, such as `[About](about.md)`. Slamhaus
resolves the complete route graph and emits site-root URLs under `base_path`.
Traversal, missing pages, route collisions, and partial links are errors.

## HTML and highlighting

Raw HTML is disabled unless `build.unsafe_html` is explicitly enabled. GFM
code fences are escaped and highlighted at build time, so generated pages do
not need a browser highlighter. Unknown languages remain escaped plain text.
