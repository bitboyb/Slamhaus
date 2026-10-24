# Themes

Slamhaus always emits a small accessible base stylesheet. Add project styles
without copying a template:

```yaml
theme:
  builtin: default
  styles:
    - assets/site.css
```

Slam UI accepts `class` properties for project styling. Generated private IDs
are implementation details and should not be used as CSS hooks.
