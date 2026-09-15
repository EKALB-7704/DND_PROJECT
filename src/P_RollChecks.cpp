#include "H_CharacterManager.h"
#include "H_ManagerHelpers.h"
#include <iostream>

// Character editor: skill checks, saving throws, ability checks and initiative.

// Every one of these is the same roll -- a d20 plus a modifier -- and the
// modifiers already exist on the character sheet. This menu is where they
// finally meet the dice roller.

namespace {

// Ability codes match the ones CharacterFeatures stores saving throws under.
const std::string kAbilityCodes[6] = {"STR", "DEX", "CON", "INT", "WIS", "CHA"};
const std::string kAbilityNames[6] = {"Strength", "Dexterity", "Constitution",
                                      "Intelligence", "Wisdom", "Charisma"};

// Modifiers are always shown with a sign, as they are on a character sheet.
std::string signedValue(int value)
{
    return (value >= 0 ? "+" : "") + std::to_string(value);
}

// Ability scores in the fixed STR/DEX/CON/INT/WIS/CHA order.
int abilityScoreAt(const Character& c, int index)
{
    switch (index)
    {
        case 0:  return c.getStrength();
        case 1:  return c.getDexterity();
        case 2:  return c.getConstitution();
        case 3:  return c.getIntelligence();
        case 4:  return c.getWisdom();
        default: return c.getCharisma();
    }
}

} // namespace

CheckRollResult CharacterManager::reportCheck(const std::string& label, int modifier)
{
    io.os() << "\n=== " << label << " (" << signedValue(modifier) << ") ===\n";
    const D20Mode mode = ManagerHelpers::promptD20Mode(io);
    const CheckRollResult result = dice.rollCheck(mode, modifier);

    // Show both rolls when advantage/disadvantage is used so the chosen result is clear.
    if (mode == D20Mode::Normal)
    {
        io.os() << "Roll: " << result.d20.chosenRoll << "\n";
    }
    else
    {
        io.os() << "Rolls: " << result.d20.firstRoll << ", " << result.d20.secondRoll << "\n";
        io.os() << "Chosen roll: " << result.d20.chosenRoll << "\n";
    }
    io.os() << "Modifier: " << signedValue(result.modifier) << "\n";
    io.os() << "Total: " << result.total << "\n";

    // Only the die matters here, not the total: a natural 20 or 1 is worth
    // calling out because some tables (and every attack roll) treat it specially.
    if (result.d20.chosenRoll == 20)
    {
        io.os() << "Natural 20!\n";
    }
    else if (result.d20.chosenRoll == 1)
    {
        io.os() << "Natural 1.\n";
    }

    return result;
}

void CharacterManager::rollChecks(Character& c)
{
    int choice = -1;

    do
    {
        io.os() << "\n=== Roll Checks And Saves ===\n";
        io.os() << "1. Skill check\n";
        io.os() << "2. Saving throw\n";
        io.os() << "3. Ability check\n";
        io.os() << "4. Initiative (" << signedValue(c.getInitiative()) << ")\n";
        io.os() << "0. Back\n";
        choice = io.readMenuChoice("Choice: ", 4);

        if (choice == 1) // Skill check
        {
            // Listed here rather than via CharacterFeatures::displaySkills,
            // which writes to std::cout and so bypasses ConsoleIO.
            const auto& skills = c.getFeatures().getSkills();
            io.os() << "\n=== Skills ===\n";
            for (size_t i = 0; i < skills.size(); i++)
            {
                io.os() << i + 1 << ". " << skills[i].name
                        << " [" << skills[i].ability << "] "
                        << signedValue(c.getSkillModifier(skills[i].name)) << "\n";
            }
            const int skillIndex = io.readInt("Skill number (0 to cancel): ", 0,
                                              static_cast<int>(skills.size()));
            if (skillIndex == 0) continue;

            const std::string& name = skills[skillIndex - 1].name;
            reportCheck(name + " check", c.getSkillModifier(name));
        }
        else if (choice == 2) // Saving throw
        {
            io.os() << "\n=== Saving Throws ===\n";
            for (int i = 0; i < 6; i++)
            {
                const bool proficient = c.getFeatures().getSaveProficiency(kAbilityCodes[i]);
                io.os() << i + 1 << ". " << (proficient ? "[P] " : "[ ] ")
                        << kAbilityNames[i] << " "
                        << signedValue(c.getSaveModifier(kAbilityCodes[i])) << "\n";
            }
            const int abilityIndex = io.readInt("Saving throw (0 to cancel): ", 0, 6);
            if (abilityIndex == 0) continue;

            reportCheck(kAbilityNames[abilityIndex - 1] + " saving throw",
                        c.getSaveModifier(kAbilityCodes[abilityIndex - 1]));
        }
        else if (choice == 3) // Ability check
        {
            // A raw ability check: the modifier alone, no proficiency.
            io.os() << "\n=== Ability Checks ===\n";
            for (int i = 0; i < 6; i++)
            {
                io.os() << i + 1 << ". " << kAbilityNames[i] << " "
                        << signedValue(Character::getAbilityModifier(abilityScoreAt(c, i))) << "\n";
            }
            const int abilityIndex = io.readInt("Ability (0 to cancel): ", 0, 6);
            if (abilityIndex == 0) continue;

            reportCheck(kAbilityNames[abilityIndex - 1] + " check",
                        Character::getAbilityModifier(abilityScoreAt(c, abilityIndex - 1)));
        }
        else if (choice == 4) // Initiative
        {
            // Initiative is stored on the sheet as a bonus, entered at creation.
            reportCheck("Initiative", c.getInitiative());
        }
    } while (choice != 0);
}
