# Deployment

## GitHub Pages

**Recommended:** commit the locally tested `dist/` directory and let the
GitHub Actions Pages job publish that directory directly. This keeps the
deployed HTML, CSS, assets, and WebAssembly modules identical to the output
you reviewed locally and avoids requiring the Emscripten toolchain in the
deployment job.

Regenerate the deployable output with:

```sh
./scripts/build-pages.sh
git add dist
git commit
```

The workflow uploads the repository root's `dist/` directory, including the
generated Doxygen reference at `/api/`. It does not upload `output/`,
`wasm-output/`, `browser-fixture/dist/`, or a build directory. `build-pages.sh`
builds the site using the configured `site.base_path`, so generated links do not
need a deployment-specific post-processing step.

### Optional Actions rebuild

The workflow can rebuild the site in GitHub Actions when started manually from
the Actions tab with `rebuild_site` enabled. This is an escape hatch for
reproducing a build in CI, not the default deployment path; it adds compiler,
Emscripten, cache, and toolchain version dependencies to deployment.

The regular `wasm` and `browser` jobs may still compile examples for validation.
Those validation outputs are separate from the Pages artifact.

For this repository, `site.base_path: /Slamhaus/` matches the project Pages URL.
Other sites should use `/` for a domain root, or set the path prefix used by
their static host, such as `/docs/v2/`. The prefix is applied consistently to
navigation, stylesheets, runtime scripts, module URLs, manifests, feeds, and
sitemaps.
