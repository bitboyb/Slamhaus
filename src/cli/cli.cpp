#include "cli/cli.hpp"

#include "core/project.hpp"
#include "language/language.hpp"
#include "lsp/lsp.hpp"
#include "toolchain/toolchain.hpp"
#include "util/process.hpp"
#include "version.hpp"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <vector>

namespace Cli {
namespace {

std::atomic<bool> watching{true};

auto Usage() -> void {
  std::cout
      << "Slamhaus " << SLAMHAUS_VERSION << "\n\n"
      << "Usage:\n"
      << "  slamhaus init [directory] [--force]\n"
      << "  slamhaus examples [directory] [--profile minimal|complete] [--force]\n"
      << "  slamhaus build [project] [-o directory] [--no-wasm] [--strict] "
         "[--dry-run] [--quiet]\n"
      << "  slamhaus check [project|file] [--strict] [--format text|json]\n"
      << "  slamhaus fmt [project|file ...] [--check]\n"
      << "  slamhaus watch [project] [-o directory] [--no-wasm] [--strict]\n"
      << "  slamhaus serve [project] [-p port] [--host address] [--no-wasm]\n"
      << "  slamhaus doctor [project] [--format text|json]\n"
      << "  slamhaus toolchain <status|install|path|remove> [--format json]\n"
      << "  slamhaus explain <diagnostic-code>\n"
      << "  slamhaus language-schema [--format json]\n"
      << "  slamhaus completion <bash|zsh|fish>\n"
      << "  slamhaus lsp\n"
      << "  slamhaus --help | --version\n";
}

auto PrintDiagnostics(const std::vector<Language::Diagnostic> &diagnostics, bool json)
    -> void {
  if (json) {
    std::cout << Language::DiagnosticsJson(diagnostics).dump(2) << '\n';
    return;
  }
  for (const auto &item : diagnostics) {
    std::cerr << item.path.generic_string() << ':' << item.range.start.line + 1 << ':'
              << item.range.start.character + 1 << ": "
              << (item.severity == Language::Severity::Error ? "error" : "warning")
              << '[' << item.code << "]: " << item.message << '\n';
    if (!item.help.empty())
      std::cerr << "  help: " << item.help << '\n';
  }
}

auto RunInit(int argc, char **argv) -> int {
  Project::InitOptions options;
  for (int i = 2; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--force")
      options.force = true;
    else if (!arg.empty() && arg.front() != '-')
      options.root = arg;
    else {
      std::cerr << "Unknown init option: " << arg << '\n';
      return 2;
    }
  }
  const auto result = Project::Init(options);
  PrintDiagnostics(result.diagnostics, false);
  if (result.success)
    std::cout << "Initialized blank Slamhaus project in "
              << std::filesystem::absolute(options.root).string() << '\n';
  return result.success ? 0 : 1;
}

auto RunExamples(int argc, char **argv) -> int {
  Project::ExampleOptions options;
  for (int index = 2; index < argc; ++index) {
    const std::string argument = argv[index];
    if (argument == "--force") {
      options.force = true;
    } else if (argument == "--profile" && index + 1 < argc) {
      const std::string profile = argv[++index];
      if (profile == "minimal")
        options.profile = Project::ExampleProfile::Minimal;
      else if (profile == "complete")
        options.profile = Project::ExampleProfile::Complete;
      else {
        std::cerr << "Profile must be minimal or complete.\n";
        return 2;
      }
    } else if (!argument.empty() && argument.front() != '-') {
      options.root = argument;
    } else {
      std::cerr << "Unknown examples option: " << argument << '\n';
      return 2;
    }
  }
  const auto result = Project::Examples(options);
  PrintDiagnostics(result.diagnostics, false);
  if (result.success)
    std::cout << "Generated Slamhaus "
              << (options.profile == Project::ExampleProfile::Minimal ? "minimal"
                                                                      : "complete")
              << " examples in " << std::filesystem::absolute(options.root).string()
              << '\n';
  return result.success ? 0 : 1;
}

auto BuildOptions(int argc, char **argv, bool watch, bool &valid) -> Project::Options {
  Project::Options options;
  valid = true;
  for (int i = 2; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--no-wasm")
      options.noWasm = true;
    else if (arg == "--strict")
      options.strict = true;
    else if (arg == "--dry-run" && !watch)
      options.dryRun = true;
    else if (arg == "--quiet" && !watch)
      options.quiet = true;
    else if (arg == "-o" && i + 1 < argc)
      options.output = argv[++i];
    else if (!arg.empty() && arg.front() != '-')
      options.root = arg;
    else {
      std::cerr << "Unknown build option: " << arg << '\n';
      valid = false;
    }
  }
  return options;
}

auto RunBuild(int argc, char **argv) -> int {
  bool valid = false;
  const auto options = BuildOptions(argc, argv, false, valid);
  if (!valid)
    return 2;
  const auto result = Project::Build(options);
  PrintDiagnostics(result.diagnostics, false);
  if (result.success && !options.quiet)
    std::cout << (options.dryRun ? "Build plan valid: " : "Built ")
              << result.outputs.size() << " files\n";
  return result.success ? 0 : 1;
}

auto Snapshot(const std::filesystem::path &root)
    -> std::map<std::filesystem::path, std::filesystem::file_time_type> {
  std::map<std::filesystem::path, std::filesystem::file_time_type> values;
  for (const auto &name : {"pages", "partials", "assets", "code"}) {
    const auto directory = root / name;
    if (!std::filesystem::exists(directory))
      continue;
    for (const auto &entry : std::filesystem::recursive_directory_iterator(directory))
      if (entry.is_regular_file())
        values[entry.path()] = entry.last_write_time();
  }
  const auto config = root / "slamhaus.yaml";
  if (std::filesystem::is_regular_file(config))
    values[config] = std::filesystem::last_write_time(config);
  return values;
}

auto RunWatch(int argc, char **argv) -> int {
  bool valid = false;
  auto options = BuildOptions(argc, argv, true, valid);
  if (!valid)
    return 2;
  std::signal(SIGINT, [](int) { watching = false; });
  std::signal(SIGTERM, [](int) { watching = false; });
  auto previous = Snapshot(options.root);
  auto result = Project::Build(options);
  PrintDiagnostics(result.diagnostics, false);
  while (watching) {
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    auto current = Snapshot(options.root);
    if (current != previous) {
      previous = std::move(current);
      result = Project::Build(options);
      PrintDiagnostics(result.diagnostics, false);
      std::cout << (result.success ? "Build complete\n" : "Build failed\n");
    }
  }
  return 0;
}

auto RunServe(int argc, char **argv) -> int {
  Project::Options options;
  std::string port = "8000";
  std::string host = "127.0.0.1";
  for (int index = 2; index < argc; ++index) {
    const std::string argument = argv[index];
    if ((argument == "-p" || argument == "--port") && index + 1 < argc)
      port = argv[++index];
    else if (argument == "--host" && index + 1 < argc)
      host = argv[++index];
    else if (argument == "--no-wasm")
      options.noWasm = true;
    else if (!argument.empty() && argument.front() != '-')
      options.root = argument;
    else {
      std::cerr << "Unknown serve option: " << argument << '\n';
      return 2;
    }
  }
  if (port.empty() || !std::all_of(port.begin(), port.end(), [](unsigned char value) {
        return std::isdigit(value) != 0;
      })) {
    std::cerr << "Port must be a number.\n";
    return 2;
  }
  const auto result = Project::Build(options);
  PrintDiagnostics(result.diagnostics, false);
  if (!result.success)
    return 1;
  auto python = Process::FindExecutable("python3");
  if (python.empty())
    python = Process::FindExecutable("python");
  if (python.empty()) {
    std::cerr << "Python is required by the development server.\n";
    return 1;
  }
  const auto output = options.output.empty()
                          ? std::filesystem::absolute(options.root) / "dist"
                          : std::filesystem::absolute(options.output);
  std::cout << "Serving " << output.string() << " at http://" << host << ':' << port
            << "/\n";
  return Process::Run(python, {"-m", "http.server", port, "--bind", host, "--directory",
                               output.string()});
}

auto RunCheck(int argc, char **argv) -> int {
  std::filesystem::path target = ".";
  bool strict = false;
  bool json = false;
  for (int i = 2; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--strict")
      strict = true;
    else if (arg == "--format" && i + 1 < argc) {
      const std::string format = argv[++i];
      if (format != "text" && format != "json") {
        std::cerr << "Unsupported diagnostic format: " << format << '\n';
        return 2;
      }
      json = format == "json";
    } else if (!arg.empty() && arg.front() != '-')
      target = arg;
    else {
      std::cerr << "Unknown check option: " << arg << '\n';
      return 2;
    }
  }
  const auto result = Project::Check(target, strict);
  PrintDiagnostics(result.diagnostics, json);
  return result.success ? 0 : 1;
}

auto RunDoctor(int argc, char **argv) -> int {
  std::filesystem::path root = ".";
  bool json = false;
  for (int i = 2; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--format" && i + 1 < argc) {
      const std::string format = argv[++i];
      if (format != "text" && format != "json") {
        std::cerr << "Unsupported diagnostic format: " << format << '\n';
        return 2;
      }
      json = format == "json";
    } else if (!arg.empty() && arg.front() != '-')
      root = arg;
    else {
      std::cerr << "Unknown doctor option: " << arg << '\n';
      return 2;
    }
  }
  const auto report = Project::Doctor(root);
  if (json)
    std::cout << report.dump(2) << '\n';
  else {
    std::cout << "Slamhaus: " << report["version"].get<std::string>() << '\n'
              << "Project: " << report["project"].get<std::string>() << '\n'
              << "Pages: " << (report["pages"].get<bool>() ? "ok" : "missing") << '\n'
              << "Config: " << (report["config"].get<bool>() ? "ok" : "missing") << '\n'
              << "Emscripten: "
              << (report["emscripten"].is_null()
                      ? "not found (optional)"
                      : report["emscripten"].get<std::string>())
              << '\n';
  }
  return report["pages"].get<bool>() ? 0 : 1;
}

auto RunFormat(int argc, char **argv) -> int {
  bool check = false;
  std::vector<std::filesystem::path> targets;
  for (int index = 2; index < argc; ++index) {
    const std::string argument = argv[index];
    if (argument == "--check")
      check = true;
    else if (!argument.empty() && argument.front() != '-')
      targets.emplace_back(argument);
    else {
      std::cerr << "Unknown fmt option: " << argument << '\n';
      return 2;
    }
  }
  if (targets.empty())
    targets.emplace_back(".");
  std::vector<std::filesystem::path> files;
  for (const auto &target : targets) {
    if (std::filesystem::is_regular_file(target)) {
      files.push_back(target);
      continue;
    }
    const auto root =
        std::filesystem::is_directory(target / "pages") ? target / "pages" : target;
    if (!std::filesystem::is_directory(root)) {
      std::cerr << "Format target does not exist: " << target.string() << '\n';
      return 2;
    }
    for (const auto &entry : std::filesystem::recursive_directory_iterator(root)) {
      const auto extension = entry.path().extension();
      if (entry.is_regular_file() && extension == ".slam")
        files.push_back(entry.path());
    }
  }
  std::sort(files.begin(), files.end());
  files.erase(std::unique(files.begin(), files.end()), files.end());
  bool changed = false;
  for (const auto &file : files) {
    std::ifstream input(file, std::ios::binary);
    const std::string source{std::istreambuf_iterator<char>(input), {}};
    if (!input.good() && !input.eof()) {
      std::cerr << "Could not read " << file.string() << '\n';
      return 1;
    }
    const auto formatted = Language::Format(source);
    if (formatted == source)
      continue;
    changed = true;
    std::cout << file.generic_string() << '\n';
    if (!check) {
      std::ofstream output(file, std::ios::binary | std::ios::trunc);
      output << formatted;
      if (!output.good()) {
        std::cerr << "Could not write " << file.string() << '\n';
        return 1;
      }
    }
  }
  return check && changed ? 1 : 0;
}

auto RunCompletion(int argc, char **argv) -> int {
  if (argc != 3) {
    std::cerr << "Usage: slamhaus completion <bash|zsh|fish>\n";
    return 2;
  }
  const std::string shell = argv[2];
  constexpr auto commands =
      "init examples build check fmt watch serve doctor toolchain explain "
      "language-schema completion lsp help version";
  if (shell == "bash") {
    std::cout << "_slamhaus(){ COMPREPLY=($(compgen -W '" << commands
              << "' -- \"${COMP_WORDS[1]}\")); }\n"
                 "complete -F _slamhaus slamhaus\n";
  } else if (shell == "zsh") {
    std::cout << "#compdef slamhaus\n_arguments '1:command:(" << commands << ")'\n";
  } else if (shell == "fish") {
    std::cout << "complete -c slamhaus -f -n '__fish_use_subcommand' -a '" << commands
              << "'\n";
  } else {
    std::cerr << "Unsupported shell: " << shell << '\n';
    return 2;
  }
  return 0;
}

auto RunToolchain(int argc, char **argv) -> int {
  if (argc < 3 || argc > 5) {
    std::cerr
        << "Usage: slamhaus toolchain <status|install|path|remove> [--format json]\n";
    return 2;
  }
  const std::string action = argv[2];
  const bool json =
      argc == 5 && std::string(argv[3]) == "--format" && std::string(argv[4]) == "json";
  if (argc > 3 && !json) {
    std::cerr << "Only toolchain status accepts --format json.\n";
    return 2;
  }
  if (action == "status") {
    const auto status = Toolchain::Status();
    if (json)
      std::cout << status.dump(2) << '\n';
    else
      std::cout << "Emscripten " << status["version"].get<std::string>() << ": "
                << (status["installed"].get<bool>() ? "installed" : "not installed")
                << '\n'
                << "Location: " << status["root"].get<std::string>() << '\n';
    return 0;
  }
  if (action == "path") {
    const auto compiler = Toolchain::Compiler();
    if (compiler.empty()) {
      std::cerr << "Managed Emscripten is not installed.\n";
      return 1;
    }
    std::cout << compiler.parent_path().parent_path().parent_path().string() << '\n';
    return 0;
  }
  const auto error = action == "install"  ? Toolchain::Install()
                     : action == "remove" ? Toolchain::Remove()
                                          : "Unknown toolchain action.";
  if (!error.empty()) {
    std::cerr << error << '\n';
    return action == "install" || action == "remove" ? 1 : 2;
  }
  std::cout << (action == "install" ? "Installed" : "Removed") << " Emscripten "
            << Toolchain::EmscriptenVersion << ".\n";
  return 0;
}

auto RunExplain(int argc, char **argv) -> int {
  if (argc != 3) {
    std::cerr << "Usage: slamhaus explain <diagnostic-code>\n";
    return 2;
  }
  static const std::map<std::string, std::string> explanations = {
      {"SH1103", "The name at this location is not a built-in Slam UI component."},
      {"SH1116", "A binding refers to a component symbol that is not declared."},
      {"SH1124", "A Slam UI call has no matching page-local C++ function."},
      {"SH1023", "YAML front matter could not be parsed."},
      {"SH3001", "A project contains C++ but no Emscripten compiler is available."},
      {"SH4010",
       "A URL has an unsafe scheme, encoded traversal, or escapes the project."},
      {"SH4011", "A local link or WASM module target does not exist."},
      {"SH4012", "Two source pages generate the same public route."},
      {"SH4027", "A configured or included partial does not exist."},
      {"SH4028", "The configured built-in theme is not available."},
      {"SH4029", "A custom stylesheet is missing or is outside assets/."},
      {"SH4021", "A permalink must begin with / and cannot contain traversal, a query, "
                 "or a fragment."}};
  std::string code = argv[2];
  std::transform(code.begin(), code.end(), code.begin(), [](unsigned char value) {
    return static_cast<char>(std::toupper(value));
  });
  const auto found = explanations.find(code);
  if (found == explanations.end()) {
    std::cerr << "Unknown diagnostic code: " << code << '\n';
    return 1;
  }
  std::cout << code << ": " << found->second << '\n';
  return 0;
}

} // namespace

auto Run(int argc, char **argv) -> int {
  if (argc < 2) {
    Usage();
    return 0;
  }
  const std::string command = argv[1];
  if (command == "--help" || command == "-h" || command == "help") {
    Usage();
    return 0;
  }
  if (command == "--version" || command == "version") {
    std::cout << "slamhaus " << SLAMHAUS_VERSION << '\n';
    return 0;
  }
  if (command == "init")
    return RunInit(argc, argv);
  if (command == "examples")
    return RunExamples(argc, argv);
  if (command == "build")
    return RunBuild(argc, argv);
  if (command == "check")
    return RunCheck(argc, argv);
  if (command == "fmt")
    return RunFormat(argc, argv);
  if (command == "watch")
    return RunWatch(argc, argv);
  if (command == "serve")
    return RunServe(argc, argv);
  if (command == "doctor")
    return RunDoctor(argc, argv);
  if (command == "toolchain")
    return RunToolchain(argc, argv);
  if (command == "explain")
    return RunExplain(argc, argv);
  if (command == "language-schema") {
    if (argc != 2 && !(argc == 4 && std::string(argv[2]) == "--format" &&
                       std::string(argv[3]) == "json")) {
      std::cerr << "Usage: slamhaus language-schema [--format json]\n";
      return 2;
    }
    std::cout << Language::SchemaJson().dump(2) << '\n';
    return 0;
  }
  if (command == "completion")
    return RunCompletion(argc, argv);
  if (command == "lsp")
    return Lsp::Run(std::cin, std::cout);
  std::cerr << "Unknown command: " << command << '\n';
  Usage();
  return 2;
}

} // namespace Cli
