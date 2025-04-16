#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace CSS
{
    std::string LoadCSS(const std::string &directoryPath)
    {
        namespace fs = std::filesystem;
        std::ostringstream combinedCSS;

        if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath))
        {
            std::cerr << "CSS directory does not exist or is not a directory: " << directoryPath << std::endl;
            return "";
        }

        for (const auto &entry : fs::recursive_directory_iterator(directoryPath, fs::directory_options::skip_permission_denied))
        {
            if (!entry.is_regular_file()) continue;

            const fs::path &path = entry.path();
            if (path.extension() == ".css")
            {
                std::ifstream cssFile(path);
                if (!cssFile.is_open())
                {
                    std::cerr << "Failed to open CSS file: " << path << std::endl;
                    continue;
                }

                combinedCSS << "/* --- " << path.filename().string() << " --- */\n";
                combinedCSS << cssFile.rdbuf() << "\n\n";
            }
        }

        return combinedCSS.str();
    }
}
