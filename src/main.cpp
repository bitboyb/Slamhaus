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
        int choice = CLI::GetMenuChoice(1, 5);

        if (choice == 5)
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
            case 3:
            {
                auto target = CLI::Prompt(
                    "\nEnter target directory for template", "my-site"
                );
                std::cout << "Cloning template into " << target << " …\n";
                if (CLI::CloneRepo(TEMPLATE_REPO, target))
                {
                    std::cout << "Template downloaded successfully.\n\n";
                }
                else
                {
                    std::cerr
                        << "Clone failed — make sure git is installed "
                        "and on your PATH.\n\n";
                }
                break;
            }
            case 4:
            {
                auto repo = CLI::Prompt(
                    "\nEnter Git repository URL"
                );
                auto target = CLI::Prompt(
                    "Enter a new target directory for your clone", "my-repo"
                );
                std::cout << "Cloning " << repo << " into " << target << " …\n";
                if (CLI::CloneRepo(repo, target))
                {
                    std::cout << "Repository cloned successfully.\n\n";
                }
                else
                {
                    std::cerr
                        << "Clone failed — ensure you specified a new directory, make sure git is installed"
                        "and on your PATH.\n\n";
                }
                break;
            }
        }
    }
    return 0;
}
