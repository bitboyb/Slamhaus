---
title: Examples
toc: true
---
# Examples

Generate a portable example that requires no WebAssembly:

```sh
slamhaus examples my-examples --profile minimal
```

Generate the complete C++, WebAssembly, mobile, and PWA example:

```sh
slamhaus examples my-examples --profile complete
cd my-examples
slamhaus toolchain install
slamhaus build
```

## Portable Markdown

Tables, task lists, autolinks, strikethrough, and fenced code use GFM. Code
highlighting is performed while the site builds, so generated pages do not
load a client-side highlighter.

## Shared programs

Small interactions can remain inside their page. Larger programs belong in
`code/<module>.cpp` or `code/<module>/*.cpp` and are selected through the
page's `runtime.module` metadata.
