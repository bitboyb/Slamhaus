#include "language/language.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>

namespace {

auto Codes(const Language::Document &document) -> std::vector<std::string> {
  std::vector<std::string> codes;
  for (const auto &diagnostic : document.diagnostics)
    codes.push_back(diagnostic.code);
  return codes;
}

auto Contains(const std::vector<std::string> &values, const std::string &value)
    -> bool {
  return std::find(values.begin(), values.end(), value) != values.end();
}

auto Count(const std::string &value, const std::string &needle) -> std::size_t {
  std::size_t result = 0;
  for (std::size_t position = 0;
       (position = value.find(needle, position)) != std::string::npos;
       position += needle.size())
    ++result;
  return result;
}

} // namespace

TEST_CASE("md remains portable GFM") {
  const auto document = Language::Parse(
      "# Markdown\n\n~~old~~\n\n- [x] shipped\n\n| A | B |\n| - | - |\n| 1 | 2 |\n",
      "page.md");
  REQUIRE_FALSE(Language::HasErrors(document.diagnostics));
  REQUIRE(document.metadata.pageTitle == "Markdown");
  const auto html = Language::Render(document).html;
  REQUIRE(html.find("<del>old</del>") != std::string::npos);
  REQUIRE(html.find("type=\"checkbox\"") != std::string::npos);
  REQUIRE(html.find("<table>") != std::string::npos);
}

TEST_CASE("md never executes enhanced fences") {
  const auto document = Language::Parse("```cpp wasm\nauto Run() -> void {}\n```\n\n"
                                        "```slam-ui\nButton Go(\"Go\");\n```\n",
                                        "portable.md");
  REQUIRE(document.behavior.source.empty());
  const auto html = Language::Render(document).html;
  REQUIRE(html.find("auto Run") != std::string::npos);
  REQUIRE(html.find("Button Go") != std::string::npos);
  REQUIRE(html.find("data-slamhaus-bindings") == std::string::npos);
}

TEST_CASE("front matter defines page and runtime metadata") {
  const auto document =
      Language::Parse("---\ntitle: Counter\ndescription: Typed\nkeywords: [cpp, wasm]\n"
                      "tags: [game, web]\ndraft: true\nruntime:\n  application: FGame\n"
                      "  canvas: Game\n---\n# Body\n",
                      "counter.slam");
  REQUIRE(document.metadata.pageTitle == "Counter");
  REQUIRE(document.metadata.pageDescription == "Typed");
  REQUIRE(document.metadata.keywords == "cpp, wasm");
  REQUIRE(document.metadata.tags == std::vector<std::string>{"game", "web"});
  REQUIRE(document.metadata.runtime.application == "FGame");
  REQUIRE(document.metadata.runtime.canvas == "Game");
  REQUIRE(document.metadata.draft);
}

TEST_CASE("front matter rejects unknown and malformed fields") {
  REQUIRE(Codes(Language::Parse("---\nlayout: post\n---\nText\n", "bad.md")) ==
          std::vector<std::string>{"SH1022"});
  REQUIRE(Codes(Language::Parse("---\ntitle: [\n---\nText\n", "bad.md")) ==
          std::vector<std::string>{"SH1023"});
  REQUIRE(Codes(Language::Parse("---\nruntime: cpp\n---\nText\n", "bad.slam")) ==
          std::vector<std::string>{"SH1023"});
}

TEST_CASE("slam ui renders typed accessible controls") {
  const auto source = R"SLAM(```slam-ui
Column Calculator() {
  TextInput Name("Name", placeholder: "Player");
  Number Score("Score", value: 10, min: 0);
  Range Volume("Volume", value: 50, max: 100);
  Checkbox Sound("Sound", checked: true);
  Select Difficulty("Difficulty", options: ["Easy", "Hard"], value: "Hard");
  TextArea Notes("Notes", rows: 4);
  Button Save("Save");
  Output Status("Ready");
};
```
)SLAM";
  const auto document = Language::Parse(source, "controls.slam");
  REQUIRE_FALSE(Language::HasErrors(document.diagnostics));
  const auto html = Language::Render(document).html;
  REQUIRE(html.find("data-slam-layout=\"column\"") != std::string::npos);
  REQUIRE(html.find("type=\"number\"") != std::string::npos);
  REQUIRE(html.find("type=\"range\"") != std::string::npos);
  REQUIRE(html.find("checked") != std::string::npos);
  REQUIRE(html.find("<option selected>Hard</option>") != std::string::npos);
  REQUIRE(html.find("aria-live=\"polite\"") != std::string::npos);
}

TEST_CASE("slam ui accepts C++-style property and event blocks") {
  const auto source = R"SLAM(```slam-ui
Column ContactForm() {
  TextInput Name("Name") {
    placeholder = "Ada";
    required = true;
  }
  Button Send("Send") {
    on Click => Submit(Name) -> Status;
  }
  Output Status("Ready");
}
```
```cpp wasm
auto Submit(FString Name) -> FString { return Name; }
```
)SLAM";
  const auto document = Language::Parse(source, "blocks.slam");
  REQUIRE_FALSE(Language::HasErrors(document.diagnostics));
  const auto html = Language::Render(document).html;
  REQUIRE(html.find("placeholder=\"Ada\"") != std::string::npos);
  REQUIRE(html.find("required") != std::string::npos);
  REQUIRE(html.find("data-slamhaus-bindings") != std::string::npos);
}

TEST_CASE("slam ui renders layout media and canvas primitives") {
  const auto source = R"SLAM(```slam-ui
Row Media() {
  Panel Preview(label: "Preview") {
    Image Poster(src: "assets/poster.png", alt: "Poster");
    Audio Music(src: "assets/music.ogg", controls: true);
    Video Trailer(src: "assets/trailer.mp4", controls: true, playsinline: true);
    Canvas Game(width: 1280, height: 720, responsive: true);
  };
};
```
)SLAM";
  const auto result = Language::Render(Language::Parse(source, "media.slam"));
  REQUIRE_FALSE(Language::HasErrors(result.diagnostics));
  REQUIRE(result.html.find("<img") != std::string::npos);
  REQUIRE(result.html.find("<audio") != std::string::npos);
  REQUIRE(result.html.find("<video") != std::string::npos);
  REQUIRE(result.html.find("data-slam-responsive") != std::string::npos);
}

TEST_CASE("bindings infer symbols events outputs and exports") {
  const auto source = R"SLAM(```slam-ui
Number Left("Left", value: 2);
Number Right("Right", value: 3);
Button Calculate("Add") => Add(Left, Right) -> Result;
Output Result("Ready");
```

```cpp wasm
auto Add(int32 A, int32 B) -> int32 { return A + B; }
```
)SLAM";
  const auto document = Language::Parse(source, "counter.slam");
  REQUIRE_FALSE(Language::HasErrors(document.diagnostics));
  REQUIRE(document.behavior.language == "cpp");
  REQUIRE(document.behavior.exports == std::vector<std::string>{"Add"});
  Language::RenderOptions options;
  options.moduleUrl = "/modules/counter.js";
  const auto html = Language::Render(document, options).html;
  REQUIRE(html.find("\"function\":\"Add\"") != std::string::npos);
  REQUIRE(html.find("#slam-left") != std::string::npos);
  REQUIRE(html.find("#slam-result") != std::string::npos);
  REQUIRE(Count(html, "slamhaus-runtime.js") == 1);
}

TEST_CASE("binding literals remain literals") {
  const auto source = R"SLAM(```slam-ui
Button Greet("Greet") => Greet("Player", 3, true) -> Result;
Output Result();
```
```cpp wasm
auto Greet(FString Name, int32 Count, bool Loud) -> FString { return Name; }
```
)SLAM";
  const auto document = Language::Parse(source, "literal.slam");
  REQUIRE_FALSE(Contains(Codes(document), "SH1116"));
  Language::RenderOptions options;
  options.moduleUrl = "/modules/literal.js";
  const auto html = Language::Render(document, options).html;
  REQUIRE(html.find("\"literal\":\"Player\"") != std::string::npos);
  REQUIRE(html.find("\"literal\":\"3\"") != std::string::npos);
}

TEST_CASE("ui grammar diagnoses unknown types properties and missing separators") {
  REQUIRE(
      Contains(Codes(Language::Parse("```slam-ui\nWidget Thing();\n```\n", "bad.slam")),
               "SH1103"));
  REQUIRE(
      Contains(Codes(Language::Parse(
                   "```slam-ui\nButton Go(\"Go\", onclick: true);\n```\n", "bad.slam")),
               "SH1106"));
  REQUIRE(Contains(Codes(Language::Parse("```slam-ui\nNumber A(\"A\" value: 1);\n```\n",
                                         "bad.slam")),
                   "SH1108"));
}

TEST_CASE("ui validation diagnoses names references and accessibility") {
  const auto source = R"SLAM(```slam-ui
Number Same("One");
Number Same("Two");
Button ("No name") => Run(Missing) -> Nowhere;
Image Logo(src: "logo.png");
```
)SLAM";
  const auto codes = Codes(Language::Parse(source, "bad.slam"));
  REQUIRE(Contains(codes, "SH1114"));
  REQUIRE(Contains(codes, "SH1115"));
  REQUIRE(Contains(codes, "SH1116"));
  REQUIRE(Contains(codes, "SH1117"));
}

TEST_CASE("interactive UI requires a behavior source or shared module") {
  const auto missing =
      Language::Parse("```slam-ui\nButton Go(\"Go\") => Run();\n```\n", "missing.slam");
  REQUIRE(Contains(Codes(missing), "SH1121"));
  const auto shared =
      Language::Parse("---\nruntime:\n  module: shared\n---\n```slam-ui\n"
                      "Button Go(\"Go\") => Run();\n```\n",
                      "shared.slam");
  REQUIRE_FALSE(Language::HasErrors(shared.diagnostics));
}

TEST_CASE("only one executable fence is accepted") {
  const auto document = Language::Parse(
      "```cpp wasm\nauto A()->void{}\n```\n```cpp wasm\nauto B()->void{}\n```\n",
      "two.slam");
  REQUIRE(Contains(Codes(document), "SH1120"));
}

TEST_CASE("future behavior adapters are reserved explicitly") {
  REQUIRE(Contains(Codes(Language::Parse("```csharp wasm\npublic class Game {}\n```\n",
                                         "game.slam")),
                   "SH1127"));
  REQUIRE(Contains(
      Codes(Language::Parse("```python wasm\ndef run(): pass\n```\n", "game.slam")),
      "SH1127"));
}

TEST_CASE("embedded source and a shared module are mutually exclusive") {
  const auto document = Language::Parse(
      "---\nruntime:\n  module: shared\n---\n```cpp wasm\nauto A()->void{}\n```\n",
      "both.slam");
  REQUIRE(Contains(Codes(document), "SH1122"));
}

TEST_CASE("application metadata names a declared canvas") {
  const auto missing =
      Language::Parse("---\nruntime:\n  application: FGame\n  canvas: Game\n---\n"
                      "```cpp wasm\nclass FGame {};\n```\n",
                      "game.slam");
  REQUIRE(Contains(Codes(missing), "SH1128"));
  const auto valid =
      Language::Parse("---\nruntime:\n  application: FGame\n  canvas: Game\n---\n"
                      "```slam-ui\nCanvas Game(width: 1280, height: 720);\n```\n"
                      "```cpp wasm\nclass FGame {};\n```\n",
                      "game.slam");
  REQUIRE_FALSE(Contains(Codes(valid), "SH1128"));
}

TEST_CASE("unclosed enhanced fences diagnose") {
  REQUIRE(Codes(Language::Parse("```slam-ui\nButton Go(\"Go\");\n", "bad.slam")) ==
          std::vector<std::string>{"SH1100"});
  REQUIRE(Codes(Language::Parse("```cpp wasm\nauto Run()->void{}\n", "bad.slam")) ==
          std::vector<std::string>{"SH1100"});
}

TEST_CASE("normal code fences are highlighted at build time") {
  const auto html =
      Language::Render(Language::Parse("```cpp\n// comment\nauto Value = 42;\n```\n"
                                       "```python\ndef run():\n    return \"ok\"\n```\n"
                                       "```csharp\npublic class Game {}\n```\n",
                                       "code.md"))
          .html;
  REQUIRE(html.find("language-cpp") != std::string::npos);
  REQUIRE(html.find("tok-comment") != std::string::npos);
  REQUIRE(html.find("tok-keyword") != std::string::npos);
  REQUIRE(html.find("tok-number") != std::string::npos);
  REQUIRE(html.find("tok-string") != std::string::npos);
}

TEST_CASE("highlighting escapes source and unknown languages remain plain") {
  const auto html =
      Language::Render(Language::Parse("```unknown\n<script>alert('x')</script>\n```\n",
                                       "escape.md"))
          .html;
  REQUIRE(html.find("&lt;script&gt;") != std::string::npos);
  REQUIRE(html.find("<script>alert") == std::string::npos);
}

TEST_CASE("headings receive stable ids and optional navigation") {
  const auto html =
      Language::Render(
          Language::Parse("---\ntoc: true\n---\n# Guide\n\n## Setup\n\n## Setup\n",
                          "guide.slam"))
          .html;
  REQUIRE(html.find("id=\"setup\"") != std::string::npos);
  REQUIRE(html.find("id=\"setup-2\"") != std::string::npos);
  REQUIRE(html.find("aria-label=\"Table of contents\"") != std::string::npos);
}

TEST_CASE("raw html is denied unless explicitly trusted") {
  const auto document = Language::Parse("Before <b>unsafe</b> after\n", "page.md");
  REQUIRE(Language::HasErrors(Language::Render(document).diagnostics));
  Language::RenderOptions options;
  options.unsafeHtml = true;
  const auto allowed = Language::Render(document, options);
  REQUIRE_FALSE(Language::HasErrors(allowed.diagnostics));
  REQUIRE(allowed.html.find("<b>unsafe</b>") != std::string::npos);
}

TEST_CASE("semantic tokens cover markdown ui and cpp regions") {
  const auto tokens =
      Language::SemanticTokens("# Heading\n```slam-ui\nButton Go(\"Go\");\n```\n"
                               "```cpp wasm\nauto Run() -> void {}\n```\n",
                               "tokens.slam");
  REQUIRE(std::any_of(tokens.begin(), tokens.end(),
                      [](const auto &token) { return token.type == "namespace"; }));
  REQUIRE(std::any_of(tokens.begin(), tokens.end(),
                      [](const auto &token) { return token.type == "type"; }));
  REQUIRE(std::any_of(tokens.begin(), tokens.end(),
                      [](const auto &token) { return token.type == "keyword"; }));
}

TEST_CASE("language schema exposes only the new public syntax") {
  const auto schema = Language::SchemaJson();
  REQUIRE(schema["schema"] == 4);
  REQUIRE(schema["fences"]["ui"] == "slam-ui");
  REQUIRE(schema["fences"]["behavior"] == "cpp wasm");
  REQUIRE(schema["syntax"]["style"] == "cpp-blocks");
  REQUIRE(schema.dump().find("@wasm") == std::string::npos);
  REQUIRE(schema.dump().find(".smd") == std::string::npos);
}

TEST_CASE("formatter is conservative and idempotent") {
  const std::string source =
      "# Prose\n\n```slam-ui\nButton Go(\"Go\") {\n  label = \"Go\";\n}\n```\n";
  REQUIRE(Language::Format(source) == source);
  REQUIRE(Language::Format(Language::Format(source)) == source);
}

TEST_CASE("link adjustment preserves query and fragment") {
  REQUIRE(Language::AdjustLink("docs/page.slam?x=1#top") == "docs/page.html?x=1#top");
  REQUIRE(Language::AdjustLink("https://example.com/a.slammer") ==
          "https://example.com/a.slammer");
}
