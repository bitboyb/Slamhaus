#pragma once

#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include <string>

namespace Generator 
{
    void GenerateSite(const std::string &contentDir, 
                      const std::string &outputDir);
}

#endif // GENERATOR_HPP
