# Slamhaus Syntax Reference - Cheat Sheet

Slamhaus extends Markdown with smart, composable tags for layout, interactivity, scripting, and more — all in a way that’s readable, reusable, and familiar. Use this page as your reference while building.

---

## 📄 Basic Markdown Support

Slamhaus fully supports **standard Markdown syntax**.

```markdown
# Heading 1
## Heading 2

- Bullet list

**Bold**, *Italic*, ***BoldItalic***

[Link Text](page.slam)
```

---

## 📦 Layout: Sections & Columns

Structure your content with readable layout blocks using `:section[]()` and `:column[]()`.

**Section Example:**

```slamhaus
:section[align:center](#about)

### Centered Section

:/section
```

**Multi-column Layout:**

```slamhaus
:column[size:3](#intro-columns)

Left Content

:column[]() 

Middle content

:column[]() 

Right content

:/column
```

---

## 🖼️ Media Embeds

Embed images, video, audio, SVG, and iframes with dedicated tags like `![alt-text](src)`, `!video[]()`, `!audio[]()`, `!svg[]()`, and `!iframe[]()`.

```slamhaus
![width:"100%" height:"300px"](assets/awesome.jpg)
![width:"50%" link:"https://example.com"](assets/logo.png)

!video[width:100% height:350px](assets/demo.mp4)
!audio[controls loop](assets/sample.mp3)
!svg[width:"64" height:"64"](assets/icon.svg)
!iframe[width:100% height:350px link:"https://www.youtube.com/embed/dQw4w9WgXcQ"]()
```

---

## 💻 Code Blocks & Tables

Slamhaus uses **standard Markdown** for code blocks and tables.

**Code Blocks:**

```cpp
#include <iostream>
int main() {
  std::cout << "Hello!";
}
```

**Tables:**

```markdown
| Feature     | Supported |
| ----------- | --------- |
| Images      | ✅        |
| Video       | ✅        |
| Audio       | ✅        |
| SVG         | ✅        |
| iFrame      | ✅        |
| Code Blocks | ✅        |
```

---

## 🔘 Buttons & Interactivity

Attach links, actions, or even C++ functions to buttons using `?button[]()`.

```slamhaus
?button[action:"mailto:hi@example.com" text:"Email Us"](#email)
?button[action:"/contact" text:"Contact Page"](#contact)
?button[action:"https://example.com" text:"Visit Site"](#external)
```

---

## 📝 Forms & Inputs

Slamhaus supports fully styled HTML forms, including inputs, textarea, and submission buttons, using `?form[]()` and nested `?input[]()`/`?textarea[]()`.

```slamhaus
?form[action:"https://formsubmit.co/you@example.com":method:"post"](#contact-form)

?input[type:"email":name:"email":placeholder:"Your Email":required:""](#email)
?input[type:"text":name:"name":placeholder:"Your Name":required:""](#name)
?textarea[name:"message":placeholder:"Your Message":rows:"6":required:""](#message)

?button[text:"Send"](#send-btn)

?/form[]()
```

---

## 🖼️ Hero Sections

Create prominent, engaging hero sections at the top of your pages using `:hero[]()`. These can feature video backgrounds, image backgrounds, or content with an image in the foreground.

**Hero with Video Background:**

This example uses a video as the background, perfect for dynamic introductions.

```slamhaus
:hero[type:"video" media:"assets/media/demo.mp4"](#landing-hero)

# Welcome to Your New Site!

This is a hero section. Introduce your site, product, or idea in one strong sentence — and direct users to learn more or take action.

?button[action:"about.html" text:"Learn More"](#home-btn)

:/hero
```

**Hero with Foreground Image:**

This hero places an image directly within the content area, alongside text and buttons.

```slamhaus
:hero[](#vscode-extension)

## 🧩 A Hero With A Picture

![width:"50%"](assets/media/demo.jpg)

Cool picture!

?button[action:"about.slam" text:"Press Me!"](#btn-example)

:/hero
```

**Hero with Background Image:**

Here, an image serves as the full background for the hero section, with text and other elements overlaid.

```slamhaus
:hero[type:"image" media:"assets/media/demo.jpg"](#vscode-extension)

## 🧩 A Hero With A Picture... As The Background!

Cool picture!

?button[action:"about.slam" text:"Press Me!"](#btn-example)

:/hero
```

---

## 🔍 Page & Site Metadata

Control SEO metadata and page-level info with `$page[]()` and `$site[]()` tags.

```slamhaus
$page[
  title:"My Page Title"
  description:"This page is awesome"
  keywords:"markdown, static site, seo"
]()

$site[
  title:"My Cool Site"
  url:"https://yoursite.com"
  favicon:"/assets/favicon.ico"
]()
```

---

## 🧠 WebAssembly via `@script[]()`

Use native C++ functions in the browser with no JavaScript by binding them to elements using `@script[]()`.

```slamhaus
@script[
  src:"/scripts/greet.js"
  bind:"#greet-btn"
  call:"Greet"
  args:"name-input"
  types:"string"
  result:"greeting-output"
]()
```

```slamhaus
@script[
  src:"/scripts/add.js"
  bind:"#add-btn"
  call:"Add"
  args:"num1,num2"
  types:"int,int"
  rtype:"number"
  result:"sum-result"
]()
```

| Attribute   | Description                     |
| :---------- | :------------------------------ |
| `src`       | Path to generated JS glue       |
| `bind`      | Element ID (e.g., `#btn`)       |
| `call`      | C++ function name               |
| `args`      | Comma-separated IDs of inputs   |
| `types`     | C++ parameter types             |
| `rtype`     | Optional return type            |
| `result`    | Element to display result       |

---

## ⚙️ Sample C++ Code (WASM)

#### **Requires [Emscripten](https://emscripten.org/)**

This is an example of the C++ code that Slamhaus compiles to WebAssembly (`.wasm`).

```cpp
#include <emscripten.h>
#include <string>

extern "C" {
  EMSCRIPTEN_KEEPALIVE
  int Add(int a, int b) {
    return a + b;
  }

  EMSCRIPTEN_KEEPALIVE
  const char* Greet(const char* name) {
    static std::string msg;
    msg = "Hello, " + std::string(name) + "!";
    return msg.c_str();
  }
}
```

Just place the `.cpp` file in your `scripts/` folder — Slamhaus will compile it to `.wasm` and generate the necessary glue code for browser use.