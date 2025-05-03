# Slamhaus

**The lightweight, Markdown-powered static site generator for developers who’d rather write than wrestle with HTML.**

Slamhaus is a modern C++ toolchain that converts plain `.md` files into fast, fully static websites. It supports an extended Markdown syntax for layouts, buttons, forms, embeds, and more—all while staying flexible, themeable, and dead simple to deploy. Whether you’re building a portfolio, documentation site, or a content-driven homepage, Slamhaus helps you get there faster.

> 💬 *Pull requests welcome! Slamhaus is open-source and open-ended—bring your ideas, features, or fixes.*

---

## 🎨 Current Features

* **Markdown In → Website Out**
  Write plain Markdown; Slamhaus handles all the HTML boilerplate.

* **Modern C++ Engine**
  Performance-focused, clean separation, familiar to game-engine devs.

* **Extended Syntax**
  Custom tags for sections, columns, buttons, forms and embeds.

* **Minimal, Tweakable Themes**
  Ships with lightweight CSS; swap or extend with your own.

* **MIT-Licensed**
  Do whatever you like—fork it, sell it, bend it to your will.

---

## 🚀 Quickstart

### Prerequisites

* **C++17-compatible compiler** (`g++`, `clang++`, MSVC…)
* **make**

### Clone & Build

Download [here](https://github.com/bitboyb/Slamhaus) or run this in your terminal:

```bash
git clone https://github.com/bitboyb/Slamhaus
cd slamhaus
make
```

This produces the `slamhaus` executable in your project root.

### Generate Your Site

You can download [this template](https://github.com/bitboyb/Slamhaus-Basic-Template) to get started!

Put the template inside your `slamhaus` directory and rename it to `content/`, then run:

```bash
./slamhaus
```

Everything in `content/` is transformed into a static site under `output/`—ready to deploy.

---

## 📂 Project Layout

```text
.
├── content/  
│   ├── assets/  
│   ├── media/        ← Images, videos and other site media.
│   └── theme/        ← CSS theme files, merged & inlined into HTML.
├── include/  
│   ├── config.ini    ← Site-wide defaults. 
│   ├── header.md     ← The site header. 
│   ├── footer.md     ← The site footer. 
│   └── nav.md        ← Navigation bar structure.
└── index.md          ← Your homepage.
```

---

## 🛠 Syntax Guide

<details>
<summary>Click to expand full syntax reference</summary>

### Basic Markdown

```markdown
# Heading 1
## Heading 2
- Bullet list
1. Numbered list
**Bold**, *Italic*, ***BoldItalic***
[Link text](page.md)
```

### Sections & Columns

```markdown
:section[align:center](#about)
### About Slamhaus
:/section
```

```markdown
:column[size:3](#features)
:column[left]()   Left column content
:column[middle]() Middle column content
:column[right]()  Right column content
:/column
```

### Images with Styling & Links

```markdown
![width:"100%"height:"300px"](assets/awesome.jpg)
![width:"50%"link:"https://example.com"](assets/logo.png)
```

### Video, Audio, SVG, iFrame

```markdown
!video[width:100% height:350px](assets/demo.mp4)
!audio[controls loop](assets/sample.mp3)
!svg[width:"64" height:"64"](assets/icon.svg)
!iframe[https://www.youtube.com/embed/dQw4w9WgXcQ]()
```

### Code Blocks

````markdown
```cpp
#include <iostream>
int main() { std::cout<<"Hello!"; }
```
````

### Tables

```
| Feature     | Supported |
| ----------- | --------- |
| Images      | ✅        |
| Video       | ✅        |
| Audio       | ✅        |
| SVG         | ✅        |
| iFrame      | ✅        |
| Code Blocks | ✅        |
```

### 🔘 Interactive Buttons

```markdown
?button[action:"mailto:hi@example.com" text:"Email Us"](#email-btn)
?button[action:"/contact" text:"Contact"](#contact-btn)
?button[action:"https://example.com" text:"Visit Site"](#visit-btn)
?button[action:"myCustomFunction" text:"Run Script"](#script-btn)
```

### 📝 Forms & Inputs

```markdown
?form[action:"https://formsubmit.co/you@example.com":method:"post"](#contact-form)

?input[type:"email":name:"email":placeholder:"Your Email":required:""](#email)
?input[type:"text": name:"name": placeholder:"Your Name":required:""](#name)
?textarea[name:"message":placeholder:"Message":rows:"6":required:""](#message)

?button[text:"Send"](#send-btn)

?/form[]()
```

### 🔍 Page & Site Metadata

```markdown
$page[title:"Home"description:"Slamhaus homepage"keywords:"markdown, static site"]()
$site[title:"Slamhaus"url:"https://slamhaus.dev"favicon:"/assets/icons/favicon.ico"]()
```

</details>
