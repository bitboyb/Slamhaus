#pragma once

#ifndef ELEMENT_HPP
#define ELEMENT_HPP

#include <string>
#include <map>

namespace Element
{
    bool IsFormOpenLine(const std::string &line);
    bool IsFormCloseLine(const std::string &line);
    bool IsButtonLine(const std::string &line);

    // std::string ParseScript(std::string &iss,
    //                         const std::string &openingLine);

    // std::string ParseForm(std::string &iss,
    //                       const std::string &openingLine);

    std::string ProcessButton(const std::string &input);
}

#endif // ELEMENT_HPP
