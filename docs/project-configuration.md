# Project configuration

`slamhaus.yaml` is the single project configuration file. Every section is
optional and unknown keys are errors.

```yaml
slamhaus:
  # Version used when this site was last checked and built.
  version: 0.1.0
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

`slamhaus.version` records the compiler version used to last verify the site.
It is intentionally a site snapshot rather than a replacement for the
repository's `VERSION` file. Review it before upgrading Slamhaus, read the
changelog for syntax and rendering changes, then run `slamhaus check --strict`,
`slamhaus fmt . --check`, and a complete build.

`build.output` must stay inside the project and may not replace `pages/`,
`partials/`, `assets/`, `code/`, or the project root. CLI `-o` overrides it.

`site.base_path` is normalized to a leading and trailing slash. Use `/` for a
domain root, or a deployment prefix such as `/Slamhaus/` or `/docs/v2/` for a
static subdirectory. It is applied to routes, canonical URLs, stylesheets,
manifests, runtime files, JavaScript modules, feeds, and sitemaps.

Layout names resolve to `.md` or `.slam` files under `partials/`. The generated
default project includes a footer linking to [slamhaus.com](https://www.slamhaus.com).
Custom styles must exist under `assets/`, and `theme.builtin` currently accepts `default`.
Optional `.slamhausignore` files at the project root exclude asset files from
the generated site. Patterns support `*` and `?`, and can match either the
asset's path or filename:

```text
# Keep source artwork out of dist/assets/
*.psd
legacy-icons/*
```
`slamhaus check` validates these references before output is written. The base
theme supplies responsive layout, form, canvas, error, and syntax-token styles.
