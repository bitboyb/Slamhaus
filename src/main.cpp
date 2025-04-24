#include <iostream>
#include "cli/cli.hpp"
#include "core/generator.hpp"

int main()
{
    std::cout << CLI::GetSlamhausLogo() << std::endl;
    std::cout << CLI::GetInfo() << std::endl;
    std::cout << "Press Any Key to Generate Site!" << std::endl;
    std::cin.get();

    std::cout << "Generating site..." << std::endl;
    Generator::GenerateSite("content", "output", "content/assets/theme/");
    std::cout << "Generation Complete!" << std::endl;

    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();
    return 0;
}
