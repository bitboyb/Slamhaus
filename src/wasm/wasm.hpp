#pragma once

#ifndef WASM_HPP
#define WASM_HPP

#include <string>
#include <vector>

namespace WASM
{
    struct WASMFile 
    {
        std::string inputPath;
        std::string outputPath;
    };
    
    bool Compile(const std::vector<WASMFile>& files, 
                 const std::string& emccPath = "emcc");
}

#endif // WASM_HPP