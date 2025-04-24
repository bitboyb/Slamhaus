$page[description:"Homepage for Slamhaus — a markdown-powered static site generator built for indie developers, technical writers, and game engine teams." keywords:"slamhaus, static site generator, markdown engine, game dev tools, documentation generator, c++ static site, custom markdown syntax, developer documentation, gamedev website, engine docs"]()

:hero[](#slamhaus-hero)

# Slamhaus!

The extendable Markdown C++ website generator.

?button[action:"slamhaus.html" text:"Learn More!"](#home-btn)

:/hero

:/hero

:section[align:center](#slamhaus-intro)

# Slamhaus!

:column[size:3](#intro-columns-0)
:column[left]()

### 🧱 
### Markdown In, Website Out

Write clean, readable content in Markdown and generate responsive websites with zero boilerplate.

:column[middle]()

### 🎮 
### Game Dev at Heart

Built with C++ in a modern game dev style — clear separation, performance-focused, and designed to feel familiar to engine developers.

:column[right]()

### 🪄 
### Extended Syntax, Your Way

Use custom Markdown-style blocks like :section[](), :column[](), and :css[]() to structure and style your content directly.

:/column

<br>

:column[size:3](#intro-columns-1)
:column[left]()

### 🎨 
### Simple Styling, No Fuss

Includes clean, minimal CSS themes out of the box — easy to tweak, extend, or replace with your own.

:column[middle]()

### 🧩 
### Free & Open Forever

MIT licensed. No strings attached. Clone it, fork it, gut it — hell, sell it if you want. Slamhaus is yours to break, build, or bend to your will, forever.

:column[right]()

### 🧠 
### Built for Tinkerers

Whether you're a solo dev, a technical writer, or making a custom docs site for your game engine — Slamhaus gives you full control without the clutter.

:/column

:/section

<br>

---

<br>

:section[align:center](#getting-started)

## 🚀 Getting Started

This guide will help you get Slamhaus up and running in minutes.  
All you need is a **C++17-compatible compiler** and the will to build something awesome.

---

### 🐧 Linux (Arch-based — *I use Arch, btw…*)

If you're on Arch, you've probably got this handled — but just in case:

```bash
sudo pacman -S base-devel
# Or just the compiler:
sudo pacman -S gcc
```

---

### 🐧 Linux (Debian-based — Ubuntu, Pop!\_OS, etc.)

For most Debian-based distros, this will install everything you need:

```bash
sudo apt update
sudo apt install g++
```

---

### 🍎 macOS

Xcode's Command Line Tools come with `clang`, `make`, and all the good stuff.  
Just run this in your terminal:

```bash
xcode-select --install
```

Alternatively, install via Homebrew:

```bash
brew install gcc make
```

---

### 🪟 Windows

Two solid options:

- ✅ **MinGW-w64** — [Download here](https://www.mingw-w64.org/)
- 🐧 **WSL (Windows Subsystem for Linux)** — [Set it up here](https://learn.microsoft.com/en-us/windows/wsl/)

We recommend WSL for a smoother Unix-style dev experience on Windows.

---

### 🛠 Building Slamhaus

Once you're in your project directory, run:

```bash
make
```

This builds the `slamhaus` executable right in your project folder.

> 💡 *No `make`? Try `mingw32-make` on Windows or install `make` via your package manager.*

---

### 🔥 Running Slamhaus

After building, generate your static site like so:

```bash
./slamhaus
```

Your content in `content/` is processed into a fully static site inside `build/`.  
Fast, portable, and ready to deploy anywhere.

---

### 🗂 Project Structure

```plaintext
.
├── content/               → Your main site content in Markdown
│   └── assets/            → Everything visual and stylistic
│       ├── icons/         → A starter icon pack to get your site rolling
│       ├── media/         → Images, videos, and other media files
│       └── theme/         → CSS and fonts for styling your site
│
├── include/               → Reusable elements injected across pages
│   ├── config.ini         → Your site’s config: title, theme, build settings, etc.
│   ├── header.md          → Content for the top of every page
│   ├── nav.md             → Navigation bar structure
│   └── footer.md          → Footer content (links, copyright, etc.)
│
└── index.md               → Your site’s entry point (home page)
```

:/section

<br>

---

<br>

:section[align:center](#syntax)

## 🛠 Syntax

Slamhaus supports extended Markdown syntax, including custom tags for layouts, rich media, and styling — all baked right into the engine.

---

### 📚 Headings & Lists

```markdown
# Heading 1
## Heading 2
### Heading 3

- Bullet list item
- Another item

1. Numbered item
2. Another one
```

---

### ✍️ Inline Formatting

*Italic*, **Bold**, and ***Bold Italic***

```markdown
*Italic*, **Bold**, and ***Bold Italic***
```

---

### 🔗 Links

```markdown
[Go to Slamhaus](index.md)
```

---

### 🖼️ Images with Styling

```markdown
![width:max height:300px Cool Banner](assets/media/awesome-image.jpg)
```

---

### 🎞️ Video Embeds

```markdown
!video[width:100% height:350px Demo Clip](assets/media/demo.mp4)
```

---

### 🔉 Audio Embeds

```markdown
!audio[controls loop](assets/media/sample-audio.mp3)
```

---

### 🖼️ SVG Support

```markdown
!svg[width:64 height:64](assets/icons/logo.svg)
```

---

### 🧱 iFrame Embeds

```markdown
!iframe[https://www.youtube.com/embed/dQw4w9WgXcQ]
```

---

### 💡 Code Highlighting

Just like regular markdown you can write code snippets and inline code.

```cpp
#include <iostream>

int main() {
    std::cout << "Hello from Slamhaus!";
    return 0;
}
```

#### 🔤 Inline Code

Write inline like `std::vector<int>` or `#include <string>` inside regular paragraphs.

---

### 📊 Tables

```markdown
| Feature       | Supported |
| ------------- | --------- |
| Images        | ✅
| Videos        | ✅
| Audio         | ✅
| SVG           | ✅
| iFrames       | ✅
| Code Blocks   | ✅
```

---

### 🧩 Custom Layouts: Columns & Sections

Slamhaus gives you layout-level control with easy-to-read tags:

```markdown
:column[size:3](#custom-layout)
:column[left]()

### Left Column Content

:column[middle]()

### Middle Column Content

:column[right]()

### Right Column Content

:/column
```

```markdown
:section[align:center](#about)

### About Slamhaus

Your Markdown content goes here, styled with custom section settings.

:/section
```

---

### 🔘 Interactive Buttons

Use `?button[]()` to create styled, scriptable buttons in your content — no raw HTML required.

```markdown
?button[action:"mailto:hello@example.com" text:"Email Us"](#email-btn)

?button[action:"/contact" text:"Go to Contact"](#contact-btn)

?button[action:"myFunction" text:"Click Me"](#js-btn)
```

- `action` — A URL (`mailto:`, `https://`, relative like `index.html`) or the name of a JavaScript function.
- `text` — Optional label text for the button (defaults to `"Submit"`).
- `(#id)` — Optional HTML ID for styling, anchors, or scripting.

---

Absolutely — here’s the updated version of the **🧠 Page & Site Metadata** section, rewritten to reflect the actual supported keys in `$site[]()` and to match your clean documentation style:

---

### 🧠 Page & Site Metadata

Add metadata to your pages with `$site[]()` and `$page[]()` blocks. These inject SEO- and share-friendly tags into the final HTML `<head>`.

#### `$page[]()` — Per-Page Metadata

```markdown
$page[
  title:"About Slamhaus"
  description:"An intro to Slamhaus, a Markdown-powered static site generator."
  keywords:"slamhaus, markdown, static site, dev docs"
]()
```

| Key           | Description                                              |
|----------------|----------------------------------------------------------|
| `title`        | Page-specific title (usually appended to site title)     |
| `description`  | Meta description for SEO and search/social previews      |
| `keywords`     | Comma-separated list of keywords for search engines      |

---

#### `$site[]()` — Global Site Metadata

```markdown
$site[
  title:"Slamhaus"
  description:"A Markdown-powered static site generator for developers."
  url:"https://slamhaus.dev"
  favicon:"/assets/icons/favicon.ico"
]()
```

| Key         | Description                                                                 |
|--------------|-----------------------------------------------------------------------------|
| `title`      | Site-wide title used in `<title>` and OpenGraph tags                       |
| `description`| Default meta description if no page-level one is set                        |
| `url`        | Canonical site URL for SEO and social embeds                                |
| `favicon`    | Path to your site’s favicon (used in `<link rel="icon">` elsewhere)         |

These values are typically defined once (e.g. in your homepage or `index.md`) and reused globally across all pages.

> 💡 You can override values from your `config.ini`, or inject these manually in special pages or custom templates.

---

## ⚡️ Enjoy Building

Slamhaus is built for developers who want **power, simplicity, and creative control** — all in one compact C++ engine.  
Whether you’re crafting documentation, building a devlog, or launching a full portfolio, Slamhaus gets out of your way and lets you ship.

**Start hacking. Stay fast. Build loud.**

:/section