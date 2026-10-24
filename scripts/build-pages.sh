#!/usr/bin/env bash
set -euo pipefail

ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
BUILD_DIR="$ROOT/build-pages"
OUTPUT_DIR="$ROOT/dist"

# Emscripten's default cache may be shared with a read-only installation.
export EM_CACHE="${EM_CACHE:-$ROOT/.emscripten_cache}"

COMPILER="$ROOT/build/slamhaus"
if [[ -x "$COMPILER" ]]; then
  cmake --build "$ROOT/build" --target slamhaus --parallel
elif [[ -x "$BUILD_DIR/slamhaus" ]]; then
  COMPILER="$BUILD_DIR/slamhaus"
fi
if [[ ! -x "$COMPILER" ]]; then
  cmake -S "$ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
  cmake --build "$BUILD_DIR" --parallel
  COMPILER="$BUILD_DIR/slamhaus"
fi

# Ensure removed pages and assets cannot survive as stale deployable files.
rm -rf "$OUTPUT_DIR"

"$COMPILER" build "$ROOT" --strict

# The compiler intentionally emits root-relative URLs. GitHub Pages project
# sites are served below /<repository>/, so make local links relative to the
# HTML file that contains them before publishing the committed artifact.
python3 - "$OUTPUT_DIR" <<'PY'
import pathlib
import re
import sys

output = pathlib.Path(sys.argv[1]).resolve()
attribute = re.compile(r'(?P<prefix>\b(?:href|src)=["\'])/(?P<url>[^"\']*)')
runtime_url = re.compile(r'(?P<prefix>["\'])/(?P<url>(?:modules/|assets/|slamhaus(?:\.css|-runtime\.js)))')

for page in output.rglob('*.html'):
    relative_dir = page.parent.relative_to(output)
    prefix = '../' * len(relative_dir.parts)

    def replace(match):
        url = match.group('url')
        if not url:
            return match.group('prefix') + (prefix or './')
        if url.startswith(('#', '/', 'http:', 'https:', 'mailto:', 'tel:')):
            return match.group(0)
        return match.group('prefix') + prefix + url

    content = page.read_text(encoding='utf-8')
    rewritten = attribute.sub(replace, content)
    rewritten = runtime_url.sub(
        lambda match: match.group('prefix') + (prefix or './') + match.group('url'),
        rewritten,
    )
    if rewritten != content:
        page.write_text(rewritten, encoding='utf-8')
PY

# Publish the generated Doxygen reference as a stable subpage of the site.
# The checked-in dist/ artifact therefore contains both the public website and
# its API documentation, while the Pages job itself only uploads dist/.
DOC_BUILD_DIR="$ROOT/build"
if [[ "$COMPILER" == "$BUILD_DIR/slamhaus" ]]; then
  DOC_BUILD_DIR="$BUILD_DIR"
fi
cmake --build "$DOC_BUILD_DIR" --target doxygen --parallel
rm -rf "$OUTPUT_DIR/api"
cp -a "$DOC_BUILD_DIR/docs-build/doxygen/html" "$OUTPUT_DIR/api"

echo "GitHub Pages output is ready in $OUTPUT_DIR (EM_CACHE=$EM_CACHE)"
