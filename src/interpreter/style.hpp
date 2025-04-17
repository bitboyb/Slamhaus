#pragma once

#ifndef STYLE_HPP
#define STYLE_HPP

#include <string>
#include <vector>

namespace Style
{
    std::string ApplyColumns(int number, 
                             const std::vector<std::string>& text, 
                             const std::vector<std::string>& openLines);

    std::string ApplyHero(const std::string &content, const std::string &openLine);
    
    bool IsColumnText(const std::string &line);
    bool IsColumnOpenLine(const std::string &line);
    bool IsColumnCloseLine(const std::string &line);
    bool IsSectionOpenLine(const std::string &line);
    bool IsSectionCloseLine(const std::string &line);
    bool IsHeroOpenLine(const std::string &line);
    bool IsHeroCloseLine(const std::string &line);

}

#endif // STYLE_HPP
