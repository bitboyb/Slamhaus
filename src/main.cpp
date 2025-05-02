#include <iostream>
#include "cli/cli.hpp"
#include "core/generator.hpp"

int main()
{
    constexpr char TEMPLATE_REPO[] = "https://github.com/your-org/slamhaus-template.git";

    std::cout << CLI::GetSlamhausLogo() << "\n\n";
    std::cout << CLI::GetInfo() << "\n\n";

    while (true)
    {
        std::cout
            << "Select an option:\n"
            << "  1) Compile current site in ./content\n"
            << "  2) Compile site from another directory\n"
            << "  3) Download basic template (" << TEMPLATE_REPO << ")\n"
            << "  4) Download template/project from a custom Git repository\n"
            << "  5) Exit\n"
            << "Enter choice [1-5]: ";

        int choice = 0;
        if (!(std::cin >> choice))
        {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Invalid input; please enter a number.\n\n";
            continue;
        }

        std::cin.ignore(10000, '\n');

        if (choice == 5)
        {
            std::cout << "Goodbye!\n";
            break;
        }

        switch (choice)
        {
        case 1:
            std::cout << "\nGenerating site from ./content …\n";
            Generator::GenerateSite("content", "output", "content/assets/theme/");
            std::cout << "Done.\n\n";
            break;

        case 2:
        {
            std::cout << "\nEnter path to your content directory: ";
            std::string dir;
            std::getline(std::cin, dir);
            if (dir.empty()) dir = "content";
            std::cout << "Generating site from " << dir << " …\n";
            Generator::GenerateSite(dir, "output", dir + "/assets/theme/");
            std::cout << "Done.\n\n";
            break;
        }

        case 3:
        {
            std::cout << "\nEnter target directory for template [my-site]: ";
            std::string target;
            std::getline(std::cin, target);
            if (target.empty()) target = "my-site";
            std::cout << "Cloning template into " << target << " …\n";
            if (CLI::CloneRepo(TEMPLATE_REPO, target))
            {
                std::cout << "Template downloaded successfully.\n\n";
            }
            else
            {
                std::cerr << "Clone failed — make sure git is installed and on your PATH.\n\n";
            }
            break;
        }

        case 4:
        {
            std::cout << "\nEnter Git repository URL: ";
            std::string repo;
            std::getline(std::cin, repo);

            std::cout << "Enter target directory [my-repo]: ";
            std::string target;
            std::getline(std::cin, target);
            if (target.empty()) target = "my-repo";

            std::cout << "Cloning " << repo << " into " << target << " …\n";
            if (CLI::CloneRepo(repo, target))
            {
                std::cout << "Repository cloned successfully.\n\n";
            }
            else
            {
                std::cerr << "Clone failed — make sure git is installed and on your PATH.\n\n";
            }
            break;
        }

        default:
            std::cout << "Please enter a number between 1 and 5.\n\n";
            break;
        }
    }

    return 0;
}
