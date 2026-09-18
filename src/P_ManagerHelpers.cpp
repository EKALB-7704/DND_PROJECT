#include "H_ManagerHelpers.h"

#include <cctype>
#include <iostream>

// Implementations of the helpers shared by the menu translation units.
namespace ManagerHelpers {

// Returns a lowercase copy so string comparisons can ignore letter casing.
std::string toLowerCopy(const std::string& value)
{
    std::string result = value;
    for (char& c : result)
    {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return result;
}

// Warlocks are the only class here whose spell slots reset on a short rest.
bool isWarlockClass(const Character& character)
{
    return toLowerCopy(character.getClass()) == "warlock";
}

std::string signedValue(int value)
{
    return (value >= 0 ? "+" : "") + std::to_string(value);
}

D20Mode promptD20Mode(ConsoleIO& io)
{
    io.os() << "1. Normal\n";
    io.os() << "2. Advantage\n";
    io.os() << "3. Disadvantage\n";
    // Previously a bad entry silently fell back to Normal; now it re-prompts.
    switch (io.readInt("Choice: ", 1, 3))
    {
        case 2:  return D20Mode::Advantage;
        case 3:  return D20Mode::Disadvantage;
        default: return D20Mode::Normal;
    }
}

SpellBook loadGlobalSpellBook(const std::filesystem::path& dataDir)
{
    SpellBook global;
    global.loadSpellBook((dataDir / "SpellBook.txt").string());
    return global;
}

void saveGlobalSpellBook(const SpellBook& spellbook, const std::filesystem::path& dataDir)
{
    spellbook.saveSpellBook((dataDir / "SpellBook.txt").string());
}

} // namespace ManagerHelpers
