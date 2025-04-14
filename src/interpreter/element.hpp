#pragma once

#ifndef ELEMENT_HPP
#define ELEMENT_HPP

#include <string>
#include <map>

namespace Element
{
    bool IsCodeLine(const std::string &line);
    bool IsButtonLine(const std::string &line);
    bool IsInputLine(const std::string &line);
    bool IsFormLine(const std::string &line);
    bool IsCodeCloseLine(const std::string &line);
    bool IsFormCloseLine(const std::string &line);

    std::string ProcessCode(const std::string &input);
    std::string ProcessButton(const std::string &input);
    std::string ProcessInputLine(const std::string &input);
    std::string ProcessFormLine(const std::string &input);
}

#endif // ELEMENT_HPP
