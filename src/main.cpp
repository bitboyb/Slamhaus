#include <iostream>
#include "core/generator.hpp"

int main()
{
    std::cout << "Generating site..." << std::endl;
    Generator::GenerateSite("content", "output", "content/assets/css/style.css");
    std::cout << "Site generated successfully!" << std::endl;
    return 0;
}
