# Documentation

## Getting Started

Slamhaus is a minimalist, C++-powered static site generator with a Markdown frontend. This guide will help you get up and running quickly. It requires a **C++17-compatible compiler**.

---

### Linux (I use Arch, btw...)

```bash
sudo pacman -S base-devel
# Or just the compiler:
sudo pacman -S gcc
```

---

### Linux (Debian-based: Ubuntu, Pop!\_OS, etc.)

```bash
sudo apt update
sudo apt install g++
```

---

### macOS

Install [Xcode Command Line Tools](https://developer.apple.com/xcode/).

```bash
xcode-select --install
```

---

### Windows

Install [MinGW-w64](https://www.mingw-w64.org/) or use [WSL](https://learn.microsoft.com/en-us/windows/wsl/) to set up a Linux environment.

---

## Building Slamhaus

Navigate to the project directory and compile it using the included Makefile:

```bash
make
```

This will generate the `slamhaus` executable inside your project folder.

---

## Running Slamhaus

After building, you can generate your website:

```bash
./slamhaus
```

This will take content from the `content/` folder and output a fully static site into the `build/` folder.

---

## Project Structure

```
├── assets/
│   ├── css/       → stylesheets and themes
│   ├── fonts/     → web fonts
│   ├── images/    → images used on site
│   └── videos/    → embedded videos
├── content/       → main site content in Markdown
├── include/
│   ├── define.md  → title of the site
│   ├── header.md  → content for the top of each page
│   ├── nav.md     → site navigation
│   └── footer.md  → footer content
└── slamhaus       → compiled executable
```

---

## Syntax

Slamhaus supports extended Markdown syntax, including some custom features.

### Headings & Lists

```markdown
# Heading 1
## Heading 2
### Heading 3

- Bullet list item
- Another item

1. Numbered item
2. Another one
```

### Inline Formatting

*Italic*, **Bold**, and ***Bold Italic***

```markdown
*Italic*, **Bold**, and ***Bold Italic***
```

### Links

[Go to Slamhaus](index.md)

```markdown
[Go to Slamhaus](index.md)
```

### Images with Styling

```markdown
![width:max height:300px Cool Banner](assets/media/awesome-image.jpg)
```

### Video Embeds

```markdown
!video[width:100% height:350px Demo Clip](assets/media/demo.mp4)
```

### iFrame Embeds

```markdown
!iframe[https://www.youtube.com/embed/dQw4w9WgXcQ]
```

---

### Code Highlighting

```cpp
#include <iostream>

int main() {
    std::cout << "Hello from Slamhaus!";
    return 0;
}
```

### Inline Code Snippets

Write inline like `std::vector<int>` or `#include <string>` in your paragraphs.

---

### Tables

```markdown
| Feature       | Supported |
| ------------- | --------- |
| Images        | ✅
| Videos        | ✅
| Tables        | ✅
| iFrames       | ✅
| Code Blocks   | ✅
```

---

## Enjoy Building

Slamhaus is designed for developers who want power, simplicity, and total control — all in one neat C++ project.

Start hacking away and turn your Markdown into a beautifully styled site.

Happy hacking!