#pragma once

#ifndef ASSET_HPP
#define ASSET_HPP

#include <string>

namespace Asset 
{
    void CopyAssets(const std::string &sourceDir, 
                    const std::string &destDir);
}

#endif // ASSET_HPP
