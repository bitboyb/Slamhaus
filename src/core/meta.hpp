#pragma once

#ifndef META_HPP
#define META_HPP

#include <vector>
#include <string>

namespace Meta
{
    void GenerateSiteMetaFiles(const std::string &siteURL,
                               const std::string &cssContent,
                               const std::string &outputDir,
                               const std::vector<std::string> &pages);
}

#endif // META_HPP