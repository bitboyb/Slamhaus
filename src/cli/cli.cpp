#include "cli.hpp"
#include <string>

namespace CLI {
std::string GetSlamhausLogo() {
  return R"(
   _____ __                __                    
  / ___// /___ _____ ___  / /_  ____ ___  _______
  \__ \/ / __ `/ __ `__ \/ __ \/ __ `/ / / / ___/
 ___/ / / /_/ / / / / / / / / / /_/ / /_/ (__  ) 
/____/_/\__,_/_/ /_/ /_/_/ /_/\__,_/\__,_/____/  
                                                                                                                                       
        )";
}

std::string GetInfo() {
  return R"(
            Creator: Syed Assad Abbas Bokhari
            Website: https://github.com/bitboyb/Slamhaus
            Version: 0.1.0
    
            License: MIT License

            CLI-first static site generation with an optional editor frontend.
        )";
}

std::string GetUsage() {
  return R"(Usage:
  slamhaus build [project-root] [--force] [--quiet]
  slamhaus editor [project-root] [--port 8090] [--no-open]
  slamhaus help

Commands:
  build   Build the project at the given root (defaults to current directory)
  editor  Launch the separate Slamhaus editor frontend for a project
  help    Show this help text
)";
}
} // namespace CLI
