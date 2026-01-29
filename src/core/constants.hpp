#pragma once

#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <string>

namespace Constants {
constexpr auto kContentDir = "content";
constexpr auto kOutputDir = "output";
constexpr auto kThemeDir = "/assets/theme";
constexpr auto kAssetsDir = "/assets/";

// File extensions
constexpr auto kExtSlam = ".slam";
constexpr auto kExtSmd = ".smd";
constexpr auto kExtHtml = ".html";
constexpr auto kExtCpp = ".cpp";
constexpr auto kExtJs = ".js";
constexpr auto kExtWasm = ".wasm";

// Meta files
constexpr auto kFileRobots = "robots.txt";
constexpr auto kFileSitemap = "sitemap.xml";
constexpr auto kFile404 = "404.html";
constexpr auto kFileHtaccess = ".htaccess";
} // namespace Constants

#endif // CONSTANTS_HPP
