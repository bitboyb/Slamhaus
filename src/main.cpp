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

  while (true) {
    CLI::ShowMenu();
    int choice = CLI::GetMenuChoice(1, 4);

    if (choice == 4) {
      Log::Info("Goodbye!");
      break;
    }

    switch (choice) {
    case 1: {
      Log::Info("\nGenerating site from ./content …");
      Generator::GenerateSite("content", "output");
      Log::Info("Done.\n");
      break;
    }
    case 2: {
      auto dir =
          CLI::Prompt("\nEnter path to your content directory", "content");
      Log::Info("Generating site from " + dir + " …");
      Generator::GenerateSite(dir, "output");
      Log::Info("Done.\n");
      break;
    }
    case 3: {
      Log::Info("\nLaunching Theme Editor...");
      auto *spec = Tools::FindToolSpec("theme-editor");
      if (spec) {
        // Defaulting to ./content as project root for this tool invocation
        // Ideally this should ask for the project root if not known, similar to
        // option 2.
        // For now, consistent with Option 1: use current dir.
        if (Tools::LaunchTool(g_toolSession, *spec, ".")) {
          // Keep running? The server runs in background.
          // The menu loop continues, so user can do other things?
          // Or should we block?
          // The prompt says "integrated into existing interactive prompt".
          // So we return to menu.
          Log::Info("Theme Editor running in background.");
        } else {
          Log::Error("Failed to launch Theme Editor.");
        }
      } else {
        Log::Error("Theme Editor tool not found in registry.");
      }
      break;
    }
    }
  }
  return 0;
}
