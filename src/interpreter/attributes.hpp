#pragma once
#ifndef ATTRIBUTES_HPP
#define ATTRIBUTES_HPP

#include <string>
#include <map>

namespace Attributes
{
    std::map<std::string, std::string> ParseAttributes(const std::string & line,
                                                       const std::string & prefix);
}

#endif // ATTRIBUTES_HPP
