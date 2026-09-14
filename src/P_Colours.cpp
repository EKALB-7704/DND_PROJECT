#include "H_Colours.h"
#include <limits>

// On Windows, opt the console into ANSI/VT100 escape-code processing.
// This is a no-op on Linux/macOS where terminals support ANSI natively.
#ifdef _WIN32
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
static void enableAnsiOnWindows()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}
#endif

// Maps the 0-15 colour codes (inherited from Windows console convention) to
// their ANSI SGR equivalents so the same codes work on both platforms.
static const char* ansiCode(int colour_code)
{
    switch (colour_code)
    {
        case BLACK:        return "\033[30m";
        case BLUE:         return "\033[34m";
        case GREEN:        return "\033[32m";
        case AQUA:         return "\033[36m";
        case RED:          return "\033[31m";
        case PURPLE:       return "\033[35m";
        case YELLOW:       return "\033[33m";
        case WHITE:        return "\033[37m";
        case GRAY:         return "\033[90m";
        case LIGHT_BLUE:   return "\033[94m";
        case LIGHT_GREEN:  return "\033[92m";
        case LIGHT_AQUA:   return "\033[96m";
        case LIGHT_RED:    return "\033[91m";
        case LIGHT_PURPLE: return "\033[95m";
        case LIGHT_YELLOW: return "\033[93m";
        case BRIGHT_WHITE: return "\033[97m";
        default:           return "\033[0m";
    }
}

ColourManager::ColourManager()
{
#ifdef _WIN32
    enableAnsiOnWindows();
#endif
}

void ColourManager::setColour(int colour_code)
{
    std::cout << ansiCode(colour_code) << std::flush;
}

void ColourManager::DisplayColourcodes()
{
    std::cout << "===Colours===" << std::endl;
    std::cout << ansiCode(BLACK)        << "0:  Black"        << ansiCode(WHITE) << std::endl;
    std::cout << ansiCode(BLUE)         << "1:  Blue"         << ansiCode(WHITE) << std::endl;
    std::cout << ansiCode(GREEN)        << "2:  Green"        << ansiCode(WHITE) << std::endl;
    std::cout << ansiCode(AQUA)         << "3:  Aqua"         << ansiCode(WHITE) << std::endl;
    std::cout << ansiCode(RED)          << "4:  Red"          << ansiCode(WHITE) << std::endl;
    std::cout << ansiCode(PURPLE)       << "5:  Purple"       << ansiCode(WHITE) << std::endl;
    std::cout << ansiCode(YELLOW)       << "6:  Yellow"       << ansiCode(WHITE) << std::endl;
    std::cout << ansiCode(WHITE)        << "7:  White"        << ansiCode(WHITE) << std::endl;
    std::cout << ansiCode(GRAY)         << "8:  Gray"         << ansiCode(WHITE) << std::endl;
    std::cout << ansiCode(LIGHT_BLUE)   << "9:  Light Blue"   << ansiCode(WHITE) << std::endl;
    std::cout << ansiCode(LIGHT_GREEN)  << "10: Light Green"  << ansiCode(WHITE) << std::endl;
    std::cout << ansiCode(LIGHT_AQUA)   << "11: Light Aqua"   << ansiCode(WHITE) << std::endl;
    std::cout << ansiCode(LIGHT_RED)    << "12: Light Red"    << ansiCode(WHITE) << std::endl;
    std::cout << ansiCode(LIGHT_PURPLE) << "13: Light Purple" << ansiCode(WHITE) << std::endl;
    std::cout << ansiCode(LIGHT_YELLOW) << "14: Light Yellow" << ansiCode(WHITE) << std::endl;
    std::cout << ansiCode(BRIGHT_WHITE) << "15: Bright White" << ansiCode(WHITE) << std::endl;
}

void ColourManager::ChangeColour(ConsoleIO& io)
{
    DisplayColourcodes();
    // Previously a bad entry printed "Colour unchanged" and bailed; readInt
    // re-prompts until the value is in range.
    const int choice = io.readInt("What Colour would you like?: ", 0, 15);
    setColour(choice);
}
