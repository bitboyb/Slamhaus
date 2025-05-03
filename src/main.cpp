#include <iostream>
#include "cli/cli.hpp"
#include "core/generator.hpp"

int main()
{
    constexpr char TEMPLATE_REPO[] =
        "https://github.com/bitboyb/Slamhaus-Basic-Template.git";

    std::cout << CLI::GetSlamhausLogo() << "\n\n"
              << CLI::GetInfo() << "\n\n";

    while (true)
    {
        CLI::ShowMenu(TEMPLATE_REPO);
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
