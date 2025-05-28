#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>

#ifdef _WIN32
    #include <windows.h>
    #include <fcntl.h>
    #include <io.h>
#endif

namespace CLI
{
    std::string GetSlamhausLogo()
    {
// #ifdef _WIN32
//     SetConsoleOutputCP(CP_UTF8);
//     _setmode(_fileno(stdout), _O_U8TEXT);
// #endif
        return R"(
   _____ __                __                    
  / ___// /___ _____ ___  / /_  ____ ___  _______
  \__ \/ / __ `/ __ `__ \/ __ \/ __ `/ / / / ___/
 ___/ / / /_/ / / / / / / / / / /_/ / /_/ (__  ) 
/____/_/\__,_/_/ /_/ /_/_/ /_/\__,_/\__,_/____/  
                                                                                                                                      
        )";
    }

    std::string GetInfo()
    {
        return R"(
            Creator: Syed Assad Abbas Bokhari
            Website: https://github.com/bitboyb/Slamhaus
            Version: 0.1.0
    
            License: MIT License
    
            Quick Start:
              1. Download a starter template and rename the folder to `content/` next to this executable:
              2. Then re-run Slamhaus to build your new site!
        )";
    }

    void ShowMenu()
    {
        std::cout
            << "Select an option:\n"
            << "  1) Compile current site in ./content\n"
            << "  2) Compile site from another directory (relative or absolute)\n"
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
