#pragma once
#include <iostream>

#include "H_ConsoleIO.h"

#define  BLACK                  0
#define  BLUE                   1
#define  GREEN                  2
#define  AQUA                   3
#define  RED                    4
#define  PURPLE                 5
#define  YELLOW                 6
#define  WHITE                  7
#define  GRAY                   8
#define  LIGHT_BLUE             9
#define  LIGHT_GREEN            10
#define  LIGHT_AQUA             11
#define  LIGHT_RED              12
#define  LIGHT_PURPLE           13
#define  LIGHT_YELLOW           14
#define  BRIGHT_WHITE           15


class Colour_manager
{
public:
    // On Windows, enables ANSI virtual terminal processing so escape codes work.
    // No-op on Linux/macOS where ANSI support is built into the terminal.
    Colour_manager();

    // Setters
    void setColour(int colour_code);

    // Display
    void DisplayColourcodes();

    // Utilities
    // Reads the colour choice through io so the prompt is validated and testable.
    void ChangeColour(ConsoleIO& io);
};
