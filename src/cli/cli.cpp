#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>

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
    
            Quick Start:
              Download a starter template and rename the folder to `content/` next to this executable:
                git clone https://github.com/bitboyb/Slamhaus-Basic-Template.git content/
              Then re-run Slamhaus to build your new site!
        )";
    }

    void ShowMenu(const std::string& templateRepo)
    {
        std::cout
            << "Select an option:\n"
            << "  1) Compile current site in ./content\n"
            << "  2) Compile site from another directory (not relative!)\n"
            << "  3) Exit\n";
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
}
