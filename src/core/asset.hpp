#pragma once

#ifndef ASSET_HPP
#define ASSET_HPP

#include <string>

namespace Asset
{
    /// Recursively copies all assets from the source directory to the destination.
    /// - Skips any `.css` files found during the copy operation
    /// - Maintains folder structure within the destination directory
    ///
    /// @param sourceDir Path to the source asset directory (e.g. ./content/assets/)
    /// @param destDir   Path to the output directory for assets (e.g. ./output/assets/)
    void CopyAssets(const std::string &sourceDir, 
                    const std::string &destDir);
}

#endif // ASSET_HPP
