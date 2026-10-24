---
title: Getting started
description: Install Slamhaus from a release binary or compile it from source, then build your first site.
toc: true
---
# Getting started

Slamhaus is a native C++17 command-line compiler. Choose a release binary when
you want to make a site quickly; compile from source when you are contributing
to Slamhaus or want to work on the compiler itself.

## Option 1: download a release binary

Download the latest package for your platform from the
[Slamhaus releases page](https://github.com/bitboyb/Slamhaus/releases/latest).
The release workflow publishes packages for Linux, macOS, and Windows when a
version tag is created.

After extracting the package, put the `slamhaus` executable on your `PATH`.
On Linux and macOS, a user-local install might look like this:

```cpp
mkdir -p "$HOME/.local/bin"
install -m 755 slamhaus "$HOME/.local/bin/slamhaus"
slamhaus --version
```

On Windows, place `slamhaus.exe` in a directory on `PATH`, or invoke it from
the extracted directory. The package also contains the public
`<slam/slam.hpp>` SDK header for C++ editor indexing and WebAssembly projects.

## Option 2: compile from source

Clone the repository and configure a Release build with CMake. A C++17 compiler,
CMake, and a working standard library are required.

```cpp
git clone https://github.com/bitboyb/Slamhaus.git
cd Slamhaus
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
cmake --install build --prefix "$HOME/.local"
slamhaus --version
```

For a development build with tests enabled:

```cpp
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

If you are changing the parser, compiler, SDK, or runtime, follow the full
verification commands in [`AGENTS.md`](https://github.com/bitboyb/Slamhaus/blob/main/AGENTS.md).

## Create your first site

The initializer creates a small universal project:

```cpp
slamhaus init my-site
cd my-site
slamhaus check --strict
slamhaus build
slamhaus serve
```

Open the local address printed by `slamhaus serve`. The generated site is in
`dist/` and can be copied to a static host, object store, CDN, or GitHub Pages.

Use `.md` for portable GFM content. Use `.slam` when a page needs Slam UI,
front-matter behavior, or an executable `cpp wasm` fence.

## Add C++ browser behavior

Install the pinned Emscripten toolchain only when a page actually needs
WebAssembly:

```cpp
slamhaus toolchain install
slamhaus build my-site
```

The [live showcase](showcase.slam) demonstrates the complete shape: named
controls, typed event bindings, readable C++, generated output, and a small
game-like interaction.

## Validate before shipping

```cpp
slamhaus check my-site --strict
slamhaus fmt my-site --check
slamhaus build my-site
```

For editor integration, run `slamhaus lsp`; for shell completion, use
`slamhaus completion bash`, `zsh`, or `fish`. Use `slamhaus doctor` when a
compiler or Emscripten installation needs diagnosis.

## Deploy the generated site

The preferred deployment path is to build locally and publish the committed
`dist/` directory. For this repository:

```cpp
./scripts/build-pages.sh
git add dist
git commit -m "Regenerate site"
git push
```

The optional GitHub Actions rebuild is useful for verification, but committed
output is the recommended release path because it matches the site tested
locally and avoids requiring the toolchain in the deployment job.
