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
generated Doxygen reference at `/api/`. It does not
upload `output/`, `wasm-output/`, `browser-fixture/dist/`, or a build directory.
`build-pages.sh` also converts local root-relative URLs to paths that work under
a GitHub Pages project URL.

### Optional Actions rebuild

The workflow can rebuild the site in GitHub Actions when started manually from
the Actions tab with `rebuild_site` enabled. This is an escape hatch for
reproducing a build in CI, not the default deployment path; it adds compiler,
Emscripten, cache, and toolchain version dependencies to deployment.

The regular `wasm` and `browser` jobs may still compile examples for validation.
Those validation outputs are separate from the Pages artifact.
