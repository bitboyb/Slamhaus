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
#include "util/log.hpp"
#include <iostream>

int main() {
  std::cout << CLI::GetSlamhausLogo() << "\n\n" << CLI::GetInfo() << "\n\n";

  while (true) {
    CLI::ShowMenu();
    int choice = CLI::GetMenuChoice(1, 3);

    if (choice == 3) {
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
    }
  }
  return 0;
}
