---
title: Recipes and examples
description: Project recipes for building Markdown sites, tools, and C++ browser experiences.
toc: true
---
# Recipes and examples

This page is about choosing a project shape and getting it shipped. For source
followed by live output, open the [live showcase](showcase.slam). For the
component and SDK lookup tables, use the [API cheatsheet](syntax.md).

## Start with a static site

```sh
slamhaus init my-site
cd my-site
slamhaus check --strict
slamhaus build
slamhaus serve
```

Use this for a portfolio, studio page, release notes, documentation, blog, or
small project site. It produces HTML and CSS with no browser runtime.

## Generate the built-in examples

The minimal profile demonstrates portable Markdown and a clean project layout:

```sh
slamhaus examples static-site --profile minimal
```

The complete profile includes typed UI, C++, WebAssembly, canvas metadata,
mobile behavior, and PWA configuration:

```sh
slamhaus examples wasm-site --profile complete
cd wasm-site
slamhaus toolchain install
slamhaus build
```

## Choose the smallest useful page

```cpp
release notes / docs / blog       -> .md
metadata or Slam UI               -> .slam
small event function              -> cpp wasm fence
growing behavior                  -> code/<module>.cpp
site-wide navigation/footer       -> partials/
interactive toy or tool           -> .slam + typed C++
```

## A practical project layout

```cpp
my-site/
├── slamhaus.yaml
├── AGENTS.md
├── pages/
│   ├── index.slam
│   └── notes.md
├── partials/
│   ├── navigation.md
│   └── footer.md
├── assets/
│   └── site.css
├── code/
│   └── tools.cpp
└── dist/
```

`slamhaus.yaml` records the compiler version, site metadata, theme styles, and
layout partials. `AGENTS.md` keeps the project's local working agreements close
to the source.

## Build a docs site

Keep the content ordinary Markdown and add front matter only where it helps:

```md
---
title: Rendering notes
description: Experiments and decisions from the graphics lab.
tags: [graphics, tools]
toc: true
---
# Rendering notes

The page remains portable GFM and can be reviewed without a compiler.
```

Add a custom stylesheet under `assets/`, then reference it from the project:

```yaml
theme:
  styles: [assets/site.css]
```

## Build a browser tool

Use the live showcase as the starting point for forms, calculators, and small
tools. The usual flow is:

1. Declare named controls and outputs in Slam UI.
2. Bind an event to a namespace-scope C++ function.
3. Run `slamhaus check --strict`.
4. Build with the pinned Emscripten toolchain.
5. Test the generated HTML and WASM module in a browser.

The compiler generates the bridge and validates argument conversion. You do not
maintain selectors, event glue, or a separate frontend build.

## Build a small game-like interaction

The [live maze](showcase.slam) demonstrates the useful split: C++ owns state and
movement rules, while Slam UI owns the board output and keyboard/mobile controls.
The same pattern works for a quiz, level editor, inventory panel, debug console,
or interactive fiction tool.

## Deploy to GitHub Pages

The repository's preferred deployment publishes committed `dist/` output:

```sh
./scripts/build-pages.sh
git add dist
git commit -m "Regenerate site"
```

The script uses the local Emscripten cache and rewrites local runtime/module
URLs for project Pages paths. GitHub Actions can rebuild manually, but committed
output is the recommended release path.

## Development loop

```sh
slamhaus check . --strict
slamhaus fmt . --check
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Use `slamhaus language-schema` and `slamhaus lsp` when integrating with an
editor. Use `slamhaus doctor` when the compiler or Emscripten toolchain needs
diagnosis.

## Why these examples are deliberately small

Slamhaus is designed for composition: readable pages, a few typed behaviors,
and a deployable result. When a project grows, move shared behavior into normal
C++ modules and keep each page's Slam UI close to the content it controls.
