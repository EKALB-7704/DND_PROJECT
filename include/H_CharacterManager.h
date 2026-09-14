#ifndef CHARACTER_MANAGER_H
#define CHARACTER_MANAGER_H

#include <vector>
#include "H_Character.h"
#include "H_ConsoleIO.h"

class CharacterManager {
private:

    // All console reads/writes go through here. Injected so menu flows can be
    // driven from a std::istringstream under test.
    ConsoleIO& io;

    std::vector<Character> characters;

    // Per-section handlers for the character editor. editCharacter() shows the
    // menu and dispatches; each of these owns one section. Private because they
    // are only ever reached through editCharacter().
    void editDetails(Character& c);
    void editHealth(Character& c);
    void editAbilityScores(Character& c);
    void editSpells(Character& c);
    void restCharacter(Character& c);
    void manageInventory(Character& c);
    void manageFeatures(Character& c);

    std::string abilityScores[6] = {"Strength: ", "Dexterity: ", "Constitution: ", "Intelligence: ", "Wisdom: ", "Charisma: "};
    std::string editCharDetailsLabels[9] = {"Name", "Race", "Class", "Background", "Alignment", "Age", "Weight", "Level", "Speed"};
    std::string editHpLabels[7] = {"Max Hp", "Current Hp", "Temporary Hp", "Hit Dice", "Roll Death Save", "Reset Death Saves", "Conditions"};

public:

    explicit CharacterManager(ConsoleIO& io);

    // Character management
    void createCharacter();
    void viewCharacters() const;
    void editCharacter();
    void manageGlobalSpells();
    bool hasCharacters() const;

    // Read-only view of the loaded roster.
    const std::vector<Character>& getCharacters() const;

    // Input validation now lives in ConsoleIO / namespace Validate.
    // Lists the loaded characters and prompts for one. Returns a 0-based index,
    // or -1 if the list is empty or the user chose 0 (Back). Replaces the
    // near-identical listing/prompt blocks that appeared in several menus.
    int selectCharacter(const std::string& prompt) const;

    // Save/Load
    void saveCharacter(const Character& c) const;
    void saveAll() const;
    void loadAll();
    std::vector<std::string> listCharacterNames() const;


};

#endif
