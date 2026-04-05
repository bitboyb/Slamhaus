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
#include "editor/editor.hpp"
#include "util/log.hpp"
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

namespace {
Editor::Session g_editorSession;

auto Shutdown() -> void {
  if (Editor::IsRunning(g_editorSession)) {
    Log::Info("Stopping editor...");
    Editor::Stop(g_editorSession);
  }
}

auto PrintUsage() -> void {
  std::cout << CLI::GetSlamhausLogo() << "\n\n" << CLI::GetInfo() << "\n\n";
  std::cout << CLI::GetUsage();
}

auto RunBuild(int argc, char **argv) -> int {
  std::filesystem::path projectRoot = ".";
  bool force = false;
  bool quiet = false;

  for (int i = 2; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--force") {
      force = true;
    } else if (arg == "--quiet") {
      quiet = true;
    } else if (!arg.empty() && arg[0] != '-') {
      projectRoot = arg;
    } else {
      Log::Error("Unknown build option: " + arg);
      return 1;
    }
  }

  return Generator::GenerateProject(projectRoot, force, quiet) ? 0 : 1;
}

auto RunEditor(int argc, char **argv) -> int {
  std::filesystem::path projectRoot = ".";
  bool openBrowser = true;
  int port = 8090;

  for (int i = 2; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--no-open") {
      openBrowser = false;
    } else if (arg == "--port") {
      if (i + 1 >= argc) {
        Log::Error("Missing port value for --port.");
        return 1;
      }
      port = std::stoi(argv[++i]);
    } else if (!arg.empty() && arg[0] != '-') {
      projectRoot = arg;
    } else {
      Log::Error("Unknown editor option: " + arg);
      return 1;
    }
  }

  if (!Editor::Launch(g_editorSession, projectRoot.string(), port,
                      openBrowser)) {
    return 1;
  }

  Log::Info("Editor running. Press Enter to stop.");
  std::string line;
  std::getline(std::cin, line);
  Editor::Stop(g_editorSession);
  return 0;
}
} // namespace

int main(int argc, char **argv) {
  std::atexit(Shutdown);

  if (argc < 2) {
    PrintUsage();
    return 0;
  }

  std::string command = argv[1];
  if (command == "build") {
    return RunBuild(argc, argv);
  }
  if (command == "editor") {
    return RunEditor(argc, argv);
  }
  if (command == "help" || command == "--help" || command == "-h") {
    PrintUsage();
    return 0;
  }

  Log::Error("Unknown command: " + command);
  PrintUsage();
  return 1;
}
