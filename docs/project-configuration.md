# Project configuration

`slamhaus.yaml` is the single project configuration file. Every section is
optional and unknown keys are errors.

```yaml
site:
  name: My Site
  url: https://example.com
  base_path: /
  language: en
  description: A static site
build:
  output: dist
  pretty_urls: true
  unsafe_html: false
theme:
  builtin: default
  styles: [assets/site.css]
layout:
  header: header
  navigation: navigation
  footer: footer
pwa:
  enabled: false
feed:
  enabled: false
```

`build.output` must stay inside the project and may not replace `pages/`,
`partials/`, `assets/`, `code/`, or the project root. CLI `-o` overrides it.

Layout names resolve to `.md` or `.slam` files under `partials/`. Custom styles
must exist under `assets/`, and `theme.builtin` currently accepts `default`.
`slamhaus check` validates these references before output is written. The base
theme supplies responsive layout, form, canvas, error, and syntax-token styles.
