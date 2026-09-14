#ifndef MANAGER_HELPERS_H
#define MANAGER_HELPERS_H

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

// Prompts for normal/advantage/disadvantage. Re-prompts until valid.
D20Mode promptD20Mode(ConsoleIO& io);

// The global spell registry, stored at data/SpellBook.txt and loaded by
// relative path -- so the program must be run from the repository root.
SpellBook loadGlobalSpellBook();
void saveGlobalSpellBook(const SpellBook& spellbook);

} // namespace ManagerHelpers

#endif
