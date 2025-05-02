
# Slamhaus

## 🎨 Features

* **Markdown In → Website Out**
  Write plain Markdown; Slamhaus handles all the HTML boilerplate.

* **Modern C++ Engine**
  Performance-focused, clean separation, familiar to game-engine devs.

* **Extended Syntax**
  Custom tags for sections, columns, buttons, forms, embeds, and CSS.

* **Minimal, Tweakable Themes**
  Ship with a lightweight CSS; swap or extend with your own.

* **MIT-Licensed**
  Do whatever you like—fork it, sell it, bend it to your will.


---

## 🚀 Quickstart

### Prerequisites
- **C++17-compatible compiler** (`g++`, `clang++`, MSVC…)
- **make**

### Clone & Build
```bash
git clone https://github.com/your-org/slamhaus.git
cd slamhaus
make
````

This produces the `slamhaus` executable in your project root.

### Generate Your Site

Put your Markdown under `content/` and run:

```bash
./slamhaus
```

Everything in `content/` is transformed into a static site under `output/`—ready to deploy!

---

## 📂 Project Layout

```text
.
.
├── content/  
│   └── assets/  
│   ├── media/        ← Images, videos and other site media assets.
│   └── theme/        ← CSS theme files, merged & inlined into your HTML.
├── include/  
│   ├── config.ini       ← Site-wide defaults. 
│   ├── header.md        ← The site header. 
│   ├── footer.md        ← The site footer. 
│   └── nav.md           ← Defining your navigation bar.
└── index.md             ← Your site’s entry-point/homepage.

```

---

## 🛠 Syntax Guide

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
Left Column Content
:column[middle]() Middle column content
Middle Column Content
:column[right]()  Right column content
Right Column Content
:/column
```

### Images with Styling & Links

```markdown
![width:"100%":height:"300px" Cool Banner](assets/awesome.jpg)
![width:"50%":link:"https://example.com" Logo](assets/logo.png)
```

### Video, Audio, SVG, iFrame

```markdown
!video[width:100% height:350px](assets/demo.mp4)
!audio[controls loop](assets/sample.mp3)
!svg[width:64 height:64](assets/icon.svg)
!iframe[https://www.youtube.com/embed/dQw4w9WgXcQ]()
```

### Code

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
| Images      | ✅         |
| Video       | ✅         |
| Audio       | ✅         |
| SVG         | ✅         |
| iFrame      | ✅         |
| Code Blocks | ✅         |

```

### Interactive Buttons
```markdown
### 🔘 Buttons

<!-- Opens the user’s mail client -->
?button[action:"mailto:hi@example.com" text:"Email Us"](#email-btn)

<!-- Navigates to an internal page -->
?button[action:"/contact" text:"Contact"](#contact-btn)

<!-- Navigates to an external URL -->
?button[action:"https://example.com" text:"Visit Site"](#visit-btn)

<!-- Calls a JavaScript function -->
?button[action:"myCustomFunction" text:"Run Script"](#script-btn)

````

### Forms & Inputs

```markdown
?form[action:"https://formsubmit.co/you@example.com":method:"post"](#contact-form)

?input[type:"email":name:"email":placeholder:"Your Email":required:""](#email)
?input[type:"text": name:"name": placeholder:"Your Name":required:""](#name)
?textarea[name:"message":placeholder:"Message":rows:"6":required:""](#message)
?button[text:"Send"](#send-btn)

?/form[]()
```

### Page & Site Metadata

```markdown
$page[
  title:"Home"
  description:"Slamhaus homepage"
  keywords:"markdown, static site"
]()

$site[
  title:"Slamhaus"
  url:"https://slamhaus.dev"
  favicon:"/assets/icons/favicon.ico"
]()
```
