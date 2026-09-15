#include "H_CharacterManager.h"
#include "H_DndExceptions.h"
#include <iostream>
#include <fstream>
#include <cctype>
#include <limits>
#include <filesystem>

// Main file for managing character utilities

// Indexing in c++ is 0-based, so first element in a container is element 0.
// But for display purposes it is converted to 1-based, and then converted back to 0-based for actual element referencing.

// All numeric values (damage, health, stats etc.) in DND are whole integers, though in the event that division occurs, such as when an attack does half damage 
// or a potion does half healing and the resulting value isnt a whole number, the numeric value is rounded up or down to a whole number at the Dungeon masters 
//discretion. This simplifies the numeric based data management, and means we dont ever have to use floats or doubles (yay)


CharacterManager::CharacterManager(ConsoleIO& io) : io(io) {}

void CharacterManager::createCharacter() {

    // Every prompt below re-prompts on bad input rather than falling through
    // with an unset or out-of-range value.
    const std::string name           = io.readName("Enter name: ");
    const std::string race           = io.readText("Enter race: ");
    const std::string characterClass = io.readText("Enter class: ");
    const std::string background     = io.readText("Enter background: ");
    const std::string alignment      = io.readText("Enter alignment: ");

    const int lvl    = io.readInt("Enter level (1-20): ", 1, 20);
    const int age    = io.readInt("Enter age: ", 0, 100000);
    const int weight = io.readInt("Enter weight: ", 0, 100000);

    const int m_hp = io.readInt("Enter max health: ", 1, 100000);
    const int c_hp = m_hp;
    const int t_hp = io.readInt("Enter temporary health (0 if none): ", 0, 100000);

    const std::string h_dice = io.readHitDice("Enter hit dice (e.g. d6, d8, d12): ");

    // Ability scores, in the fixed STR/DEX/CON/INT/WIS/CHA order.
    int new_AS[6];
    for (int i = 0; i < 6; i++)
    {
        new_AS[i] = io.readInt("Enter " + abilityScores[i], 1, 20);
    }

    const int init = io.readInt("Enter initiative: ", -10, 20);
    // Proficiency bonus is +2 at level 1 and never lower; +6 is the level-20 cap.
    const int prof = io.readInt("Enter proficiency (2-6): ", 2, 6);

    // Build character using constructor and place into vector
    characters.emplace_back(name, race, characterClass, background, alignment,
                            lvl, age, weight, c_hp, m_hp, t_hp, h_dice,
                            new_AS[0], new_AS[1], new_AS[2],
                            new_AS[3], new_AS[4], new_AS[5], init, prof);

    io.os() << "Character created!\n";
}

bool CharacterManager::hasCharacters() const
{
    return !characters.empty();
}

const std::vector<Character>& CharacterManager::getCharacters() const
{
    return characters;
}

int CharacterManager::selectCharacter(const std::string& prompt) const {
    if (characters.empty()) {
        io.os() << "No characters available.\n";
        return -1;
    }

    io.os() << "\n=== Characters ===\n";
    for (size_t i = 0; i < characters.size(); i++)
        io.os() << i + 1 << ". " << characters[i].getName() << "\n";
    io.os() << "0. Back\n";

    const int choice = io.readMenuChoice(prompt, static_cast<int>(characters.size()));
    return choice == 0 ? -1 : choice - 1;
}

void CharacterManager::viewCharacters() const {
    const int index = selectCharacter("Choice: ");
    if (index < 0) return;

    // Show all details of chosen character
    characters[index].display();
}

void CharacterManager::editCharacter() {
    // Check for characters
    if (characters.empty()) {
        io.os() << "No characters to edit.\n";
        return;
    }

    const int index = selectCharacter("Enter character index: ");
    if (index < 0) return;

    // Assigns the character object chose to reference c to apply edits
    Character& c = characters[index];

    int choice;

    // Display edit menu
    do {
        io.os() << "\n=== " << c.getName() << " Editor ===\n";
        io.os() << "1. Character details \n2. Character health \n3. Inventory \n4. Ability scores \n5. Spells \n6. Features and skills \n7. Rest (short / long) \n8. Roll checks and saves \n0. Back \n";
        choice = io.readMenuChoice("Choice: ", 8);

        if (choice == 1) // Character details
        {
            editDetails(c);
        }
        else if (choice == 2) // Health
        {
            editHealth(c);
        }
        else if (choice == 3) // Inventory
        {
            manageInventory(c);
        }
        else if (choice == 4) // Ability scores
        {
            editAbilityScores(c);
        }
        else if (choice == 5) // Spells and short/long rest
        {
            editSpells(c);
        }
        else if (choice == 6) // Feats and abilites
        {
            manageFeatures(c);
        }
        else if (choice == 7) // Short / long rest
        {
            restCharacter(c);
        }
        else if (choice == 8) // Skill checks, saving throws, ability checks, initiative
        {
            rollChecks(c);
        }
        else if (choice == 0)
        {
            io.os() << "Exiting edit menu " << std::endl;
        }

    } while (choice != 0);
}

// Save/Load functions
void CharacterManager::saveCharacter(const Character& c) const {
    namespace fs = std::filesystem;
    fs::path dir = fs::path("data") / "characters" / c.getName();
    try {
        fs::create_directories(dir);
        c.saveToDirectory(dir.string());
        io.os() << c.getName() << " saved.\n";
    } catch (const SaveError& e) {
        io.os() << "Failed to save " << c.getName() << ": " << e.what() << "\n";
    } catch (const fs::filesystem_error& e) {
        io.os() << "Filesystem error saving " << c.getName() << ": " << e.what() << "\n";
    }
}

void CharacterManager::saveAll() const {
    if (characters.empty()) {
        io.os() << "No characters to save.\n";
        return;
    }
    for (const auto& c : characters)
        saveCharacter(c);
}

void CharacterManager::loadAll() {
    namespace fs = std::filesystem;
    fs::path base = fs::path("data") / "characters";

    if (!fs::exists(base) || !fs::is_directory(base)) {
        io.os() << "No saved characters found.\n";
        return;
    }

    characters.clear();
    int loaded = 0;
    try {
        for (const auto& entry : fs::directory_iterator(base)) {
            if (entry.is_directory() && fs::exists(entry.path() / "character.txt")) {
                try {
                    // Repairs are surfaced rather than applied silently, so a
                    // character that loads with bad data is visibly flagged.
                    std::vector<std::string> repairs;
                    characters.push_back(
                        Character::loadFromDirectory(entry.path().string(), &repairs));
                    loaded++;
                    if (!repairs.empty()) {
                        io.os() << "Repaired " << entry.path().filename().string() << ":\n";
                        for (const auto& note : repairs) {
                            io.os() << "  - " << note << "\n";
                        }
                        io.os() << "  Save this character to write the corrected data.\n";
                    }
                } catch (const LoadError& e) {
                    io.os() << "Skipped " << entry.path().filename().string()
                              << ": " << e.what() << "\n";
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        io.os() << "Filesystem error reading character directory: " << e.what() << "\n";
    }

    if (loaded == 0)
        io.os() << "No saved characters found.\n";
    else
        io.os() << "Loaded " << loaded << " character(s).\n";
}

std::vector<std::string> CharacterManager::listCharacterNames() const {
    namespace fs = std::filesystem;
    std::vector<std::string> names;
    fs::path base = fs::path("data") / "characters";

    if (!fs::exists(base) || !fs::is_directory(base))
        return names;

    for (const auto& entry : fs::directory_iterator(base)) {
        if (entry.is_directory())
            names.push_back(entry.path().filename().string());
    }
    return names;
}
