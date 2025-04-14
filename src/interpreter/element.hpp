#pragma once

#ifndef ELEMENT_HPP
#define ELEMENT_HPP

#include <string>
#include <map>

namespace Element
{
    std::string Trim(const std::string& str);
    
    bool IsScriptOpenLine(const std::string &line);
    bool IsScriptCloseLine(const std::string &line);
    bool IsFormOpenLine(const std::string &line);
    bool IsFormCloseLine(const std::string &line);
    bool IsButtonLine(const std::string &line);

    std::string ParseScript(std::string &iss,
                            const std::string &openingLine);

    std::string ParseForm(std::string &iss,
                          const std::string &openingLine);

    std::string ProcessButton(const std::string &input);
}

#endif // ELEMENT_HPP
