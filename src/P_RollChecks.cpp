#include "H_CharacterManager.h"
#include "H_ManagerHelpers.h"
#include "H_Weapon.h"
#include <iostream>
#include <vector>

// Character editor: skill checks, saving throws, ability checks, initiative
// and weapon attacks.

// Every one of these is the same roll -- a d20 plus a modifier -- and the
// modifiers already exist on the character sheet. This menu is where they
// finally meet the dice roller.

namespace {

// Ability codes match the ones CharacterFeatures stores saving throws under.
const std::string kAbilityCodes[6] = {"STR", "DEX", "CON", "INT", "WIS", "CHA"};
const std::string kAbilityNames[6] = {"Strength", "Dexterity", "Constitution",
                                      "Intelligence", "Wisdom", "Charisma"};

using ManagerHelpers::signedValue;

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
        io.os() << "\n=== Roll Checks, Saves And Attacks ===\n";
        io.os() << "1. Skill check\n";
        io.os() << "2. Saving throw\n";
        io.os() << "3. Ability check\n";
        io.os() << "4. Initiative (" << signedValue(c.getInitiative()) << ")\n";
        io.os() << "5. Weapon attack\n";
        io.os() << "0. Back\n";
        choice = io.readMenuChoice("Choice: ", 5);

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
        else if (choice == 5) // Weapon attack
        {
            attackWithWeapon(c);
        }
    } while (choice != 0);
}

// Attack roll, then damage on a hit.
//
// The attack is a d20 check like any other: the weapon's ability modifier,
// plus proficiency if the character is proficient with it. Damage adds the
// same ability modifier but never proficiency. A natural 20 always hits and
// doubles the damage dice; a natural 1 always misses. Anything in between
// depends on the target's AC, which only the table knows, so it asks.
void CharacterManager::attackWithWeapon(Character& c)
{
    // Weapons are listed by position among the weapons, not the whole
    // inventory, so the numbering has no gaps where armor or gear sits.
    std::vector<const Weapon*> weapons;
    for (int i = 1; i <= c.getInventory().size(); i++)
    {
        if (const auto* weapon = dynamic_cast<const Weapon*>(&c.getInventory().getItem(i)))
        {
            weapons.push_back(weapon);
        }
    }

    if (weapons.empty())
    {
        io.os() << "No weapons in the inventory. Add one under Inventory first.\n";
        return;
    }

    io.os() << "\n=== Weapons ===\n";
    for (size_t i = 0; i < weapons.size(); i++)
    {
        io.os() << i + 1 << ". " << weapons[i]->getName() << " - "
                << weapons[i]->getDamageDice() << " " << weapons[i]->getDamageType()
                << " [" << signedValue(c.getWeaponAbilityModifier(*weapons[i])) << "]\n";
    }
    const int weaponIndex = io.readInt("Weapon (0 to cancel): ", 0,
                                       static_cast<int>(weapons.size()));
    if (weaponIndex == 0) return;

    const Weapon& weapon = *weapons[weaponIndex - 1];
    const int abilityMod = c.getWeaponAbilityModifier(weapon);

    // The sheet does not record weapon proficiencies, so ask each time.
    const bool proficient = io.readYesNo("Proficient with " + weapon.getName() + "? (y/n): ");
    const int attackMod = abilityMod + (proficient ? c.getProficiency() : 0);

    const CheckRollResult attack = reportCheck(weapon.getName() + " attack", attackMod);

    const bool critical = attack.d20.chosenRoll == 20;
    if (attack.d20.chosenRoll == 1)
    {
        io.os() << "The attack misses.\n";
        return;
    }
    if (critical)
    {
        io.os() << "Critical hit: damage dice are doubled.\n";
    }
    else if (!io.readYesNo("Does " + std::to_string(attack.total) +
                           " hit the target's AC? (y/n): "))
    {
        io.os() << "The attack misses.\n";
        return;
    }

    io.os() << "\n=== " << weapon.getName() << " damage ===\n";

    DiceExpression expr;
    if (!parseDiceExpression(weapon.getDamageDice(), expr))
    {
        // Free-text damage the parser does not understand, such as a
        // versatile "1d8/1d10": fall back to a total rolled at the table.
        const int damage = io.readInt("Could not roll \"" + weapon.getDamageDice() +
                                      "\". Enter the damage rolled: ", 0, 100000);
        io.os() << "Damage: " << damage << " " << weapon.getDamageType() << "\n";
        return;
    }

    const DamageRollResult damage = dice.rollDamage(expr, abilityMod, critical);
    if (!damage.rolls.empty())
    {
        io.os() << "Damage rolls: ";
        for (size_t i = 0; i < damage.rolls.size(); i++)
        {
            io.os() << damage.rolls[i] << (i + 1 < damage.rolls.size() ? ", " : "\n");
        }
    }
    io.os() << "Modifier: " << signedValue(damage.modifier) << "\n";
    io.os() << "Damage: " << damage.total << " " << weapon.getDamageType() << "\n";
}
