# Third-party notices

Slamhaus binaries statically include the following pinned open-source projects:

- cmark-gfm, BSD-2-Clause.
- nlohmann/json, MIT.
- yaml-cpp, MIT.
- Tree-sitter, MIT.
- tree-sitter-cpp, MIT.
- tree-sitter-python, MIT.
- tree-sitter-c-sharp, MIT.

Catch2 (BSL-1.0) is used only by the test executable. Exact versions are pinned
in `CMakeLists.txt`; upstream license files are included in fetched source trees
during source builds.
