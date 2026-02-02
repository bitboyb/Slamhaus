// ----------------------------------------
// Slamhaus 0.1.0
// https://github.com/bitboyb/Slamhaus
// MIT Licensed
//
//   _____ __                __
//   / ___// /___ _____ ___  / /_  ____ ___  _______
//   \__ \/ / __ `/ __ `__ \/ __ \/ __ `/ / / / ___/
//  ___/ / / /_/ / / / / / / / / / /_/ / /_/ (__  )
// /____/_/\__,_/_/ /_/ /_/_/ /_/\__,_/\__,_/____/
//
//
// ----------------------------------------

#include "cli/cli.hpp"
#include "core/generator.hpp"
#include "tools/tools.hpp"
#include "util/log.hpp"
#include <iostream>

// Global session for cleanup
Tools::ToolSession g_toolSession;

void Shutdown() {
  if (Tools::IsToolRunning(g_toolSession)) {
    Log::Info("Stopping active tool...");
    Tools::StopTool(g_toolSession);
  }
}

int main() {
  std::atexit(Shutdown);
  // Simple signal handling could be added here for robust cleanup

  std::cout << CLI::GetSlamhausLogo() << "\n\n" << CLI::GetInfo() << "\n\n";

  // Auto-Launch Dashboard
  Log::Info("Launching Slamhaus Dashboard...");
  if (Tools::LaunchTool(g_toolSession, ".")) {
    Log::Info("Dashboard running at http://localhost:8090");
    Log::Info("Use the Web Interface to Build, Edit, and Manage your project.");
  } else {
    Log::Error("Failed to launch dashboard.");
    return 1;
  }

  // Simple keep-alive loop
  while (true) {
    // We could just sleep, or provide a minimal text menu
    std::cout << "\n[Server Running] Type 'q' to exit, 'o' to open browser: ";
    std::string input;
    std::cin >> input;

    if (input == "q" || input == "exit") {
      break;
    }
    if (input == "o") {
      Tools::OpenBrowser("http://localhost:8090/");
    }
  }

  return 0;
}
