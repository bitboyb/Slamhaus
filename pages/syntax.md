---
title: Slamhaus API cheatsheet
description: A compact reference for Slamhaus components, properties, events, and SDK helpers.
toc: true
---
# Slamhaus API cheatsheet

Use the [examples](examples.md) as the guided, live version of this reference.
This page is the quick lookup sheet.

## Page files

| File | Purpose |
| --- | --- |
| `.md` | Portable GitHub Flavored Markdown |
| `.slam` | Markdown plus Slam UI and optional executable C++ |
| `partials/` | Reusable Markdown or Slamhaus regions |
| `code/` | Shared C++ modules |
| `assets/` | CSS, fonts, images, audio, and other public files |
| `dist/` | Complete generated site |

## Front matter

```yaml
---
title: Page title
description: Search description
tags: [one, two]
draft: false
noindex: false
toc: true
permalink: /custom/route/
runtime:
  module: shared-module
  application: FGame
  canvas: Game
---
```

## Slam UI grammar

```cpp
Type Name("Label") {
  property = value;
  on Event => Function(Input, Other) -> Output;
  Child("Label");
};
```

Values: strings, numbers, booleans, identifiers, and lists.

## Components

| Family | Components |
| --- | --- |
| Layout | `Row`, `Column`, `Panel`, `Form` |
| Inputs | `TextInput`, `Number`, `Range`, `Checkbox`, `Select`, `TextArea` |
| Actions | `Button` |
| Output | `Text`, `Output` |
| Media | `Image`, `Audio`, `Video` |
| Surface | `Canvas` |
| Composition | `Include` |
| Semantic | `Element` with safe tags |

## Common properties

| Property | Values | Used by |
| --- | --- | --- |
| `value` | string or number | inputs |
| `placeholder` | string | text inputs |
| `required` | boolean | form inputs |
| `min`, `max`, `step` | number | number/range |
| `checked` | boolean | checkbox |
| `options` | list | select |
| `src` | local or external URL | media |
| `alt` | string | image |
| `controls` | boolean | audio/video |
| `playsinline` | boolean | video |
| `width`, `height` | number | canvas |
| `responsive` | boolean | canvas |
| `class` | string | any component |
| `tag` | safe semantic tag | `Element` |

## Default events

| Component | Default event |
| --- | --- |
| `Button` | `Click` |
| `Form` | `Submit` |
| `Range` | `Input` |
| Other controls | `Change` |

Override them with `on Event => Function(...) -> Output;`.

## Typed behavior

```cpp wasm
auto Add(int32 A, int32 B) -> int32 { return A + B; }
```

Supported boundary values include `FString`, `bool`, fixed-width integers,
floating-point values, and `void`. Functions are namespace-scope and cannot be
overloaded when exported from a page.

## SDK quick reference

| Type | Purpose |
| --- | --- |
| `FInput` | Keyboard, mouse, pointer, touch, and gamepads |
| `FCanvas` | Canvas size, fullscreen, and pointer lock |
| `FDisplay` | Viewport, device scale, orientation |
| `FDocument` | Safe text, values, classes, visibility, and focus |
| `FStorage` | Local and session storage |
| `FAudio` | Play, pause, stop, volume, resume |
| `FHttpClient` | Async HTTP requests |
| `FAssetManager` | Async text asset loading |
| `FNavigation` | Internal and external navigation |
| `FAccessibility` | Announcements and reduced-motion preference |
| `FSensors` | Orientation and acceleration |
| `FPwa` | Install capability and prompt |
| `FApplication` | `BeginPlay`, `Tick`, `EndPlay`, suspend/resume, resize |

## CLI loop

```sh
slamhaus init my-site
slamhaus check my-site --strict
slamhaus fmt my-site --check
slamhaus build my-site
slamhaus serve my-site
```

Use `slamhaus check . --format json` for tooling, `slamhaus language-schema` for
editor integrations, and `slamhaus lsp` for the editor-neutral language server.
