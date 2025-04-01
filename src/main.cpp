#include <iostream>
#include "generator.hpp"

int main()
{
    std::cout << "Generating site..." << std::endl;
    GenerateSite("content", "output", "assets/css/style.css");
    std::cout << "Site generated successfully!" << std::endl;
    return 0;
}
