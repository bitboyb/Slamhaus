#pragma once

#ifndef ID_HPP
#define ID_HPP

#include <string>

namespace ID
{
    std::string ExtractIDTag(const std::string& line);
    std::string ApplyIDTag(const std::string& htmlLine, const std::string& markdownLine);

    bool isIDTag(const std::string &line);
}

#endif // ID_HPP
