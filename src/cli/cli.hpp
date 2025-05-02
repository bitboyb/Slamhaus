#pragma once
#ifndef CLI_HPP
#define CLI_HPP

#include <string>

namespace CLI
{
    std::string GetSlamhausLogo();
    std::string GetInfo();
    void ShowMenu(const std::string& templateRepo);
    int GetMenuChoice(int min, int max);
    std::string Prompt(const std::string& question,
                       const std::string& defaultValue = "");
    bool CloneRepo(const std::string& repoUrl,
                   const std::string& targetDir);
}

#endif // CLI_HPP
