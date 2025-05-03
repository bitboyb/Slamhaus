#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>

namespace CLI
{
    std::string GetSlamhausLogo()
    {
        return R"(
   ▄████████  ▄█          ▄████████   ▄▄▄▄███▄▄▄▄      ▄█    █▄       ▄████████ ███    █▄     ▄████████ 
  ███    ███ ███         ███    ███ ▄██▀▀▀███▀▀▀██▄   ███    ███     ███    ███ ███    ███   ███    ███ 
  ███    █▀  ███         ███    ███ ███   ███   ███   ███    ███     ███    ███ ███    ███   ███    █▀  
  ███        ███         ███    ███ ███   ███   ███  ▄███▄▄▄▄███▄▄   ███    ███ ███    ███   ███        
▀███████████ ███       ▀███████████ ███   ███   ███ ▀▀███▀▀▀▀███▀  ▀███████████ ███    ███ ▀███████████ 
         ███ ███         ███    ███ ███   ███   ███   ███    ███     ███    ███ ███    ███          ███ 
   ▄█    ███ ███▌    ▄   ███    ███ ███   ███   ███   ███    ███     ███    ███ ███    ███    ▄█    ███ 
 ▄████████▀  █████▄▄██   ███    █▀   ▀█   ███   █▀    ███    █▀      ███    █▀  ████████▀   ▄████████▀  
             ▀                                                                                          
        )";
    }

    std::string GetInfo()
    {
        return R"(
        Creator: Syed Assad Abbas Bokhari
        Website: https://www.piledriver-playhouse.com/slamhaus
        Version: 0.1.0

        License: MIT License
        Copyright (c) 2025 Piledriver Playhouse Limited
        )";
    }

    void ShowMenu(const std::string& templateRepo)
    {
        std::cout
            << "Select an option:\n"
            << "  1) Compile current site in ./content\n"
            << "  2) Compile site from another directory (not relative!)\n"
            << "  3) Download basic template (" << templateRepo << ")\n"
            << "  4) Download template/project from a custom Git repository\n"
            << "  5) Exit\n";
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

    bool CloneRepo(const std::string& repoUrl,
                   const std::string& targetDir)
    {
        if (targetDir.empty())
        {
            std::cerr << "Error: no target directory provided.\n";
            return false;
        }
        std::filesystem::path dest{targetDir};
        auto parent = dest.parent_path();
        if (!parent.empty() && !std::filesystem::exists(parent))
        {
            std::error_code ec;
            if (!std::filesystem::create_directories(parent, ec))
            {
                std::cerr << "Error: unable to create directory \""
                          << parent.string() << "\": " << ec.message() << "\n";
                return false;
            }
        }
        std::ostringstream cmd;
        cmd << "git clone \"" << repoUrl << "\" \"" << dest.string() << "\"";
        int rc = std::system(cmd.str().c_str());
        if (rc != 0)
        {
            std::cerr << "Clone failed (exit code " << rc << "). "
                      << "Make sure git is installed and you have write access to:\n    "
                      << dest.string() << "\n";
            return false;
        }

        return true;
    }
    
}
