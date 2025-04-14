#pragma once

#ifndef ID_HPP
#define ID_HPP

#include <string>

namespace ID
{
    std::string ApplyIDTag(const std::string& line);

    bool isIDTag(const std::string &line);
}

#endif // ID_HPP
