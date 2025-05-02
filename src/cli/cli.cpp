#include <iostream>
#include <string>
#include <sstream>

namespace CLI
{
    std::string GetSlamhausLogo()
    {
        return R"(
   ▄████████  ▄█          ▄████████   ▄▄▄▄███▄▄▄▄      ▄█    █▄       ▄████████ ███    █▄     ▄████████ 
  ███    ███ ███         ███    ███ ▄██▀▀▀███▀▀▀██▄   ███    ███     ███    ███ ███    ███   ███    ███ 
  ███    █▀  ███         ███    ███ ███   ███   ███   ███    ███     ███    ███ ███    ███   ███    █▀  
  ███        ███         ███    ███ ███   ███   ███  ▄███▄▄▄▄███▄▄   ███    ███ ███    ███   ███        
▀███████████ ███       ▀███████████ ███   ███   ███ ▀▀███▀▀▀▀███▀  ▀███████████ ███    ███ ▀███████████ 
         ███ ███         ███    ███ ███   ███   ███   ███    ███     ███    ███ ███    ███          ███ 
   ▄█    ███ ███▌    ▄   ███    ███ ███   ███   ███   ███    ███     ███    ███ ███    ███    ▄█    ███ 
 ▄████████▀  █████▄▄██   ███    █▀   ▀█   ███   █▀    ███    █▀      ███    █▀  ████████▀   ▄████████▀  
             ▀                                                                                          
        )";
    }

    std::string GetInfo()
    {
        return R"(
        Creator: Syed Assad Abbas Bokhari
        Website: https://www.piledriver-playhouse.com/slamhaus
        Version: 0.1.0

        License: MIT License
        Copyright (c) 2025 Piledriver Playhouse Limited
        )";
    }

    void ShowMenu(const std::string& templateRepo)
    {
        std::cout
            << "Select an option:\n"
            << "  1) Compile current site in ./content\n"
            << "  2) Compile site from another directory (not relative!)\n"
            << "  3) Download basic template (" << templateRepo << ")\n"
            << "  4) Download template/project from a custom Git repository\n"
            << "  5) Exit\n";
    }

    int GetMenuChoice(int min, 
                      int max)
    {
        while (true)
        {
            std::cout << "Enter choice [" << min << "-" << max << "]: ";
            int choice;
            if (std::cin >> choice && choice >= min && choice <= max)
            {
                std::cin.ignore(10000, '\n');
                return choice;
            }
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << " ➞ Invalid; please enter a number between "
                    << min << " and " << max << ".\n";
        }
    }

    std::string Prompt(const std::string& question,
                       const std::string& defaultValue)
    {
        std::cout << question;
        if (!defaultValue.empty())
        {
            std::cout << " [" << defaultValue << "]";
        }
        std::cout << ": ";
        std::string line;
        std::getline(std::cin, line);
        return line.empty() ? defaultValue : line;
    }

    bool CloneRepo(const std::string& repoUrl,
                   const std::string& targetDir)
    {
        std::ostringstream cmd;
        cmd << "git clone \"" << repoUrl << "\" \"" << targetDir << "\"";
        return std::system(cmd.str().c_str()) == 0;
    }
    
}
