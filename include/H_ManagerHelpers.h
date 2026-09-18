#ifndef MANAGER_HELPERS_H
#define MANAGER_HELPERS_H

#include <filesystem>
#include <string>

#include "H_Character.h"
#include "H_ConsoleIO.h"
#include "H_DiceRoller.h"
#include "H_SpellBook.h"

// Small helpers shared by the CharacterManager menu translation units.
//
// These lived in an anonymous namespace in P_CharacterManager.cpp while every
// menu was in that one file. Splitting the menus across files means they need
// real linkage, so they live here rather than being duplicated.
namespace ManagerHelpers {

// Lowercase copy, so string comparisons can ignore letter casing.
std::string toLowerCopy(const std::string& value);

// Warlocks are the only class here whose spell slots reset on a short rest.
bool isWarlockClass(const Character& character);

// Modifiers are always shown with a sign, as they are on a character sheet.
std::string signedValue(int value);

// Prompts for normal/advantage/disadvantage. Re-prompts until valid.
D20Mode promptD20Mode(ConsoleIO& io);

// The global spell registry, stored as SpellBook.txt in the data folder.
SpellBook loadGlobalSpellBook(const std::filesystem::path& dataDir);
void saveGlobalSpellBook(const SpellBook& spellbook, const std::filesystem::path& dataDir);

} // namespace ManagerHelpers

#endif
