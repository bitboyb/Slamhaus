#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include <string>

void GenerateSite(const std::string &contentDir, 
                  const std::string &outputDir, 
                  const std::string &templatePath, 
                  const std::string &cssPath);

#endif // GENERATOR_HPP
