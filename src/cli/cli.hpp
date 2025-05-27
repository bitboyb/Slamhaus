#pragma once

#ifndef CLI_HPP
#define CLI_HPP

#include <string>

namespace CLI
{
    /// Returns the Slamhaus ASCII logo string.
    /// - Compatible with UTF-8 terminals (Windows-safe).
    std::string GetSlamhausLogo();

    /// Returns version, author, license, and quickstart instructions.
    std::string GetInfo();

    /// Displays the interactive CLI menu to the terminal.
    void ShowMenu();

    /// Prompts user to enter a number between [min, max].
    /// Repeats until valid input is received.
    ///
    /// @param min Minimum valid number (inclusive)
    /// @param max Maximum valid number (inclusive)
    /// @return The selected number
    int GetMenuChoice(int min, int max);

    /// Prompts the user for input with optional default value.
    ///
    /// @param question      The message to display
    /// @param defaultValue  The fallback value if input is blank
    /// @return The entered value or default
    std::string Prompt(const std::string& question,
                       const std::string& defaultValue = "");
}

#endif // CLI_HPP
