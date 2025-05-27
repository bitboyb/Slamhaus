#pragma once

#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include <string>

namespace Generator 
{
    /// Generates the full static site from the given content directory.
    /// - Parses all `.md` files into HTML with template and metadata injection
    /// - Compiles `.cpp` files in-place into WebAssembly using Emscripten
    /// - Copies assets, injects CSS, builds sitemap, robots.txt, and 404 page
    ///
    /// @param contentDir Path to your Markdown source directory (e.g. ./content)
    /// @param outputDir  Destination for the compiled static site (e.g. ./output)
    void GenerateSite(const std::string &contentDir, 
                      const std::string &outputDir);
}

#endif // GENERATOR_HPP
