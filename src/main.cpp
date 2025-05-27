// ----------------------------------------
// Slamhaus 0.1.0
// https://github.com/bitboyb/Slamhaus
// MIT Licensed
// 
//   _____ __                __                    
//   / ___// /___ _____ ___  / /_  ____ ___  _______
//   \__ \/ / __ `/ __ `__ \/ __ \/ __ `/ / / / ___/
//  ___/ / / /_/ / / / / / / / / / /_/ / /_/ (__  ) 
// /____/_/\__,_/_/ /_/ /_/_/ /_/\__,_/\__,_/____/  
//
//
// ----------------------------------------

#include <iostream>
#include "cli/cli.hpp"
#include "core/generator.hpp"

int main()
{
    std::cout << CLI::GetSlamhausLogo() << "\n\n"
              << CLI::GetInfo() << "\n\n";

    while (true)
    {
        CLI::ShowMenu();
        int choice = CLI::GetMenuChoice(1, 3);

        if (choice == 3)
        {
            std::cout << "Goodbye!\n";
            break;
        }

        switch (choice)
        {
            case 1:
            {
                std::cout << "\nGenerating site from ./content …\n";
                Generator::GenerateSite(
                    "content",
                    "output"
                );
                std::cout << "Done.\n\n";
                break;
            }
            case 2:
            {
                auto dir = CLI::Prompt(
                    "\nEnter path to your content directory", "content"
                );
                std::cout << "Generating site from " << dir << " …\n";
                Generator::GenerateSite(
                    dir,
                    "output"
                );
                std::cout << "Done.\n\n";
                break;
            }
        }
    }
    return 0;
}
